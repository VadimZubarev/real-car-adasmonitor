#include <iostream>
#include <obd_parser.h>
#include "onnx_classifier_wrapper.h"

int main() {
    OBDParser parser;
    
    try {
        parser.parseCSVFile("../data/data.csv");
        int slow, normal, aggressive, unknown;
        int n = 5;

        for (int i = 0; i < n; i++) {
            parser.printRecord(i);
        }

        parser.getStyleStatistics(n, slow, normal, aggressive, unknown);
        
        std::cout << "SLOW: " << slow << std::endl;
        std::cout << "NORMAL: " << normal << std::endl;
        std::cout << "AGGRESSIVE: " << aggressive << std::endl;
        std::cout << "UNKNOWN: " << unknown << std::endl;
        
        // parser.printAllRecords();        
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }


    // Загрузите первые 20 записей из CSV, классифицируйте каждую, выведите таблицу:
    // истинная метка – предсказание – уверенность. Подсчитайте точность.
    // git commit -m "feat: ONNX classifier module"

    try {
        parser.parseCSVFile("../data/data.csv");
    
        ONNXClassifier classifier(
            "../models/driver_classifier.onnx",
            "../models/normalization_params.json"
        );
    
        int n = 1000;
        int count = 0;
    
        int correct = 0;
    
        std::cout << "\nTRUE\tPRED\tCONFIDENCE\n";
        std::cout << "---------------------------------\n";
    
        for (int i = 0; i < n; i++)
        {
            OBDRecord record = parser.getOBDRecord(i);

            // Пропускаем записи с аномальными значениями
            if (record.getCoolantTemp() < 30 || record.getCoolantTemp() > 130) {
                continue;  // температура вне диапазона
            }
            if (record.getFuelLevel() < 5 || record.getFuelLevel() > 100) {
                continue;  // топливо вне диапазона
            }
    
            DrivingStyle true_label = record.getStyle();
            count++;
    
            std::vector<float> input = {
                static_cast<float>(record.getRPM()),
                static_cast<float>(record.getSpeed()),
                static_cast<float>(record.getThrottlePos()),
                static_cast<float>(record.getCoolantTemp()),
                static_cast<float>(record.getFuelLevel())
            };
    
            ClassificationResult result = classifier.predict(input);
    
            std::cout
                << static_cast<int>(true_label) << "\t"
                << static_cast<int>(result.label) << "\t"
                << result.confidence << "\n";
    
            if (static_cast<int>(result.label) == static_cast<int>(true_label))
                correct++;
        }
    
        float accuracy = static_cast<float>(correct) / count;
    
        std::cout << "---------------------------------\n";
        std::cout << "Accuracy: " << accuracy << std::endl;
        std::cout << "Correct: " << correct << std::endl;
    
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}