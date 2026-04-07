#include "obd_parser.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

OBDRecord::OBDRecord()
    : timestamp(0)
    , rpm(0)
    , speed(0)
    , throttle_pos(0.0)
    , coolant_temp(0.0)
    , fuel_level(0.0)
    , style(DrivingStyle::UNKNOWN)
{}

time_t OBDRecord::getTimestamp() const { return timestamp; }
int OBDRecord::getRPM() const { return rpm; }
int OBDRecord::getSpeed() const { return speed; }
double OBDRecord::getThrottlePos() const { return throttle_pos; }
double OBDRecord::getCoolantTemp() const { return coolant_temp; }
double OBDRecord::getFuelLevel() const { return fuel_level; }
DrivingStyle OBDRecord::getStyle() const { return style; }

void OBDRecord::setTimestamp(time_t ts) { timestamp = ts; }
void OBDRecord::setRPM(int r) { rpm = r; }
void OBDRecord::setSpeed(int s) { speed = s; }
void OBDRecord::setThrottlePos(double tp) { throttle_pos = tp; }
void OBDRecord::setCoolantTemp(double ct) { coolant_temp = ct; }
void OBDRecord::setFuelLevel(double fl) { fuel_level = fl; }
void OBDRecord::setStyle(DrivingStyle st) { style = st; }

std::string OBDParser::drivingStyleToString(DrivingStyle style) const {
    switch (style) {
        case DrivingStyle::SLOW:
            return "SLOW";
        case DrivingStyle::NORMAL:
            return "NORMAL";
        case DrivingStyle::AGGRESSIVE:
            return "AGGRESSIVE";
        default:
            return "UNKNOWN";
    }
}

DrivingStyle OBDParser::parseStyle(const std::string& str, const DrivingStyle defaultStyle)  const {
    if (str == "SLOW") return DrivingStyle::SLOW;
    if (str == "NORMAL") return DrivingStyle::NORMAL;
    if (str == "AGGRESSIVE") return DrivingStyle::AGGRESSIVE;
    return defaultStyle;
}

size_t OBDParser::getRecordCount() const {
    return obd_records.size();
}

bool OBDParser::isEmpty() const {
    return obd_records.empty();
}

void OBDParser::clear() {
    obd_records.clear();
}

OBDRecord OBDParser::getOBDRecord(int index) const {
    if (index < 0 || index >= static_cast<int>(obd_records.size())) {
        throw std::out_of_range("Index out of range: " + std::to_string(index));
    }
    return obd_records[index];
}

// Безопасное преобразование в double
double OBDParser::safeStod(const std::string& str, double defaultValue) const {
    if (str.empty()) {
        return defaultValue;
    }
    try {
        return std::stod(str);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Cannot convert '" << str << "' to double, using default" << std::endl;
        return defaultValue;
    }
}

// Безопасное преобразование в int
int OBDParser::safeStoi(const std::string& str, int defaultValue) const {
    if (str.empty()) {
        return defaultValue;
    }
    try {
        return std::stoi(str);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Cannot convert '" << str << "' to int, using default" << std::endl;
        return defaultValue;
    }
}

// Безопасное преобразование в long long
long long OBDParser::safeStoll(const std::string& str, long long defaultValue) const {
    if (str.empty()) {
        return defaultValue;
    }
    try {
        return std::stoll(str);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Cannot convert '" << str << "' to long long, using default" << std::endl;
        return defaultValue;
    }
}



void OBDParser::parseRecord(const std::string& line) {
    if (line.empty()) {
        return;
    }
    
    std::stringstream ss(line);
    std::string value;
    std::vector<std::string> values;
    
    // Разделяем по запятой
    while (std::getline(ss, value, ',')) {
        // Удаляем пробелы в начале и конце
        size_t start = value.find_first_not_of(" \t\r\n");
        size_t end = value.find_last_not_of(" \t\r\n");
        
        if (start == std::string::npos) {
            values.push_back("");
        } else {
            values.push_back(value.substr(start, end - start + 1));
        }
    }
    
    // Нужно минимум 7 полей: timestamp, RPM, SPEED, THROTTLE_POS, COOLANT_TEMP, FUEL_LEVEL, STYLE
    if (values.size() != 7) {
        std::cerr << "Warning: Invalid line format (only " << values.size() 
                  << " fields), skipping..." << std::endl;
        return;
    }
    
    OBDRecord record;
    
    try {
        // Поля в порядке: timestamp, RPM, SPEED, THROTTLE_POS, COOLANT_TEMP, FUEL_LEVEL, STYLE
        // 0: timestamp, 1: RPM, 2: SPEED, 3: THROTTLE_POS, 4: COOLANT_TEMP, 5: FUEL_LEVEL, 6: Style
        record.setTimestamp(static_cast<time_t>(safeStoll(values[0], 0)));
        record.setRPM(safeStoi(values[1], 0));
        record.setSpeed(safeStoi(values[2], 0));
        record.setThrottlePos(safeStod(values[3], 0.0));
        record.setCoolantTemp(safeStod(values[4], 0.0));
        record.setFuelLevel(safeStod(values[5], 0.0));
        record.setStyle(parseStyle(values[6], DrivingStyle::UNKNOWN));
        
        obd_records.push_back(record);
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing line: " << e.what() << std::endl;
        std::cerr << "Problem line: " << line << std::endl;
    }
}

int OBDParser::parseCSVFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return -1;
    }
    
    std::string line;
    bool isFirstLine = true;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Пропускаем заголовок (первую строку)
        if (isFirstLine) {
            isFirstLine = false;
            std::cout << "Skipping header: " << line << std::endl;
            continue;
        }
        
        if (line.empty()) {
            continue;
        }
        
        parseRecord(line);
    }
    
    file.close();
    std::cout << "Loaded " << getRecordCount() << " records from " << filename << std::endl;
    
    int parseResult = static_cast<int>(getRecordCount());
    return parseResult;
}

void OBDParser::printRecord(int index) const {
    if (index < 0 || index >= static_cast<int>(obd_records.size())) {
        std::cerr << "Index out of range" << std::endl;
        return;
    }
    
    const auto& record = obd_records[index];
    std::cout << "=== OBD Record " << index << " ===" << std::endl;
    std::cout << "Timestamp: " << record.getTimestamp() << std::endl;
    std::cout << "RPM: " << record.getRPM() << std::endl;
    std::cout << "Speed: " << record.getSpeed() << " km/h" << std::endl;
    std::cout << "Throttle Position: " << std::fixed << std::setprecision(2) 
              << record.getThrottlePos() << "%" << std::endl;
    std::cout << "Coolant Temp: " << record.getCoolantTemp() << "°C" << std::endl;
    std::cout << "Fuel Level: " << record.getFuelLevel() << "%" << std::endl;
}

void OBDParser::printAllRecords() const {
    std::cout << "=== Total Records: " << obd_records.size() << " ===" << std::endl;
    for (size_t i = 0; i < obd_records.size(); ++i) {
        printRecord(static_cast<int>(i));
        std::cout << "------------------------" << std::endl;
    }
}

void OBDParser::getStyleStatistics(int n, int& slowCount, int& normalCount, int& aggressiveCount, int& unknownCount) const {
    slowCount = 0;
    normalCount = 0;
    aggressiveCount = 0;
    unknownCount = 0;

    size_t limit = static_cast<size_t>(n);
    if (limit > obd_records.size()) {
        limit = obd_records.size();
    }
    
    for (size_t i = 0; i < limit; ++i) {
        const auto& record = obd_records[i];
        switch (record.getStyle()) {
            case DrivingStyle::SLOW:
                slowCount++;
                break;
            case DrivingStyle::NORMAL:
                normalCount++;
                break;
            case DrivingStyle::AGGRESSIVE:
                aggressiveCount++;
                break;
            default:
                unknownCount++;
                break;
        }
    }
}