#include "onnx_classifier_wrapper.h"
#include "../onnx_wrapper/wrapper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>

std::vector<float> ONNXClassifier::extractArray(const std::string& text, const std::string& key)
{
    size_t key_pos = text.find("\"" + key + "\"");
    if (key_pos == std::string::npos)
        throw std::runtime_error("Key not found: " + key);

    size_t start = text.find('[', key_pos);
    size_t end = text.find(']', start);

    if (start == std::string::npos || end == std::string::npos)
        throw std::runtime_error("Invalid JSON array for key: " + key);

    std::string array_str = text.substr(start + 1, end - start - 1);

    std::vector<float> result;
    std::stringstream ss(array_str);

    float value;
    while (ss >> value)
    {
        result.push_back(value);
        if (ss.peek() == ',')
            ss.ignore();
    }

    return result;
}

void ONNXClassifier::loadNormalization(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open normalization file: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    // Извлекаем mean и std из JSON
    mean = extractArray(text, "mean");
    std_dev = extractArray(text, "std");

    // Проверяем размер (должно быть 5 признаков)
    if (mean.size() != std_dev.size()) {
        throw std::runtime_error("Mean and std have different sizes");
    }

    std::cout << "Loaded normalization: " << mean.size() << " features" << std::endl;
    
    std::cout << "Mean: ";
    for (auto m : mean) std::cout << m << " ";
    std::cout << std::endl;
}


ONNXClassifier::ONNXClassifier(const std::string& model_path,
                               const std::string& normalization_json_path)
{
    handle = ort_create_session(model_path.c_str());
    if (!handle) {
        throw std::runtime_error("Failed to create ONNX session. Check model path: " + model_path);
    }

    // normalization json
    loadNormalization(normalization_json_path);
}

ONNXClassifier::~ONNXClassifier()
{
    ort_release_session(handle);
}

ClassificationResult ONNXClassifier::predict(const std::vector<float>& telemetry)
{
    std::cout << "predict called\n";
    if (!handle) {
        throw std::runtime_error("ONNX session not initialized");
    }

    // нормализация
    std::vector<float> normalized(telemetry.size());
    for (size_t i = 0; i < telemetry.size(); i++) {
        float s = std_dev[i];
        if (s == 0.0f) s = 1e-6f;  // защита от деления на ноль
        normalized[i] = (telemetry[i] - mean[i]) / s;
    }
    
    float logits[3];
    
    ort_predict(handle, normalized.data(), logits, telemetry.size());
    
    // softmax
    float probs[3];
    
    float max_logit = logits[0];
    for (int i = 1; i < 3; i++) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 3; i++) {
        probs[i] = std::exp(logits[i] - max_logit);
        sum += probs[i];
    }
    
    for (int i = 0; i < 3; i++) {
        probs[i] /= sum;
    }
    
    // выбрать лучший
    ClassificationResult result;
    result.scores = {probs[0], probs[1], probs[2]};
    
    int best_class = 0;
    float best_prob = probs[0];
    for (int i = 1; i < 3; i++) {
        if (probs[i] > best_prob) {
            best_prob = probs[i];
            best_class = i;
        }
    }
    
    result.label = static_cast<ClassificationResult::Label>(best_class);
    result.confidence = best_prob;
    
    return result;
}