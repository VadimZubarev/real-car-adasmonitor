#pragma once
#include <array>
#include <vector>
#include <string>

// Результат остаётся таким же — это твой контракт
struct ClassificationResult {
    enum Label {
        AGGRESSIVE = 0,
        NORMAL = 1,
        SLOW = 2
    };

    Label label;
    float confidence;
    std::array<float, 3> scores;
};

class ONNXClassifier {
private:
    void* handle;
    std::vector<float> mean;
    std::vector<float> std_dev;

public:
    // загрузка модели происходит ВНУТРИ DLL
    ONNXClassifier(const std::string& model_path,
                   const std::string& normalization_json_path);

    ~ONNXClassifier();

    // единственная функция инференса
    ClassificationResult predict(const std::vector<float>& telemetry);

private:
    void loadNormalization(const std::string& path);
    static std::vector<float> extractArray(const std::string& text, const std::string& key);
};