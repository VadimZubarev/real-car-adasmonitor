#ifndef OBD_PARSER_H
#define OBD_PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <ctime>

enum class DrivingStyle {
    SLOW,
    NORMAL,
    AGGRESSIVE,
    UNKNOWN
};

class OBDRecord {
private:
    // Временные метки
    time_t timestamp;

    // обороты двигателя
    int rpm;

    // скорость км/ч
    int speed;

    // положение дроссельной заслонки %
    double throttle_pos;              
    
    // температура охлаждающей жидкости °C
    double coolant_temp;
    
    // уровень топлива %
    double fuel_level; 

    // стиль вождения
    DrivingStyle style;

public:
    OBDRecord();
    
    virtual ~OBDRecord() = default;
    
    time_t getTimestamp() const;
    int getRPM() const;
    int getSpeed() const;
    double getThrottlePos() const;
    double getCoolantTemp() const;
    double getFuelLevel() const;
    DrivingStyle getStyle() const;
    
    void setTimestamp(time_t ts);
    void setRPM(int r);
    void setSpeed(int s);
    void setThrottlePos(double tp);
    void setCoolantTemp(double ct);
    void setFuelLevel(double fl);
    void setStyle(DrivingStyle s);
};

class OBDParser {
private:
    std::vector<OBDRecord> obd_records;
    
public:
    OBDRecord getOBDRecord(int index) const;

    size_t getRecordCount() const;

    int parseCSVFile(const std::string& filename);

    void parseRecord(const std::string&);

    void printRecord(int index) const;

    void printAllRecords() const;

    bool isEmpty() const;

    void clear();

    // Вспомогательные методы для безопасного парсинга
    double safeStod(const std::string& str, double defaultValue = 0.0) const;
    int safeStoi(const std::string& str, int defaultValue = 0) const;
    long long safeStoll(const std::string& str, long long defaultValue = 0) const;
    DrivingStyle parseStyle(const std::string& str, DrivingStyle style = DrivingStyle::UNKNOWN) const;
    std::string drivingStyleToString(DrivingStyle style) const;

    void getStyleStatistics(int n, int& slowCount, int& normalCount, int& aggressiveCount, int& unknownCount) const;
};

#endif // OBD_PARSER_H