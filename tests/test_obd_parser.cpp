#include <fstream>
#include <cstdio> 

#include <gtest/gtest.h>
#include "../src/obd_parser.h"

class OBDParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        obj = new OBDParser();
    }
    
    void TearDown() override {
        delete obj;
    }
    
    OBDParser* obj;
};

TEST_F(OBDParserTest, ParseStyle) {
    EXPECT_EQ(obj->parseStyle("SLOW", DrivingStyle::UNKNOWN), DrivingStyle::SLOW);
    EXPECT_EQ(obj->parseStyle("NORMAL", DrivingStyle::UNKNOWN), DrivingStyle::NORMAL);
    EXPECT_EQ(obj->parseStyle("AGGRESSIVE", DrivingStyle::UNKNOWN), DrivingStyle::AGGRESSIVE);
    EXPECT_EQ(obj->parseStyle("UNKNOWN", DrivingStyle::UNKNOWN), DrivingStyle::UNKNOWN);
    EXPECT_EQ(obj->parseStyle("AAABBB", DrivingStyle::UNKNOWN), DrivingStyle::UNKNOWN);
}

TEST_F(OBDParserTest, LoadFile) {
    EXPECT_EQ(obj->parseCSVFile("a.csv"), -1);
    EXPECT_EQ(obj->parseCSVFile("data.txt"), -1);
}
TEST_F(OBDParserTest, GetOBDRecord) {
    EXPECT_EQ(obj->getRecordCount(), 0);
    
    int parseResult = obj->parseCSVFile("../data/data.csv");
    
    EXPECT_GT(parseResult, 0);
    EXPECT_EQ(obj->getRecordCount(), parseResult);
    
    if (parseResult > 0) {
        EXPECT_NO_THROW(obj->getOBDRecord(0));
        EXPECT_NO_THROW(obj->getOBDRecord(parseResult - 1));
    }
    
    EXPECT_THROW(obj->getOBDRecord(-1), std::out_of_range);
    EXPECT_THROW(obj->getOBDRecord(parseResult), std::out_of_range);
    EXPECT_THROW(obj->getOBDRecord(parseResult + 100), std::out_of_range);
}

TEST_F(OBDParserTest, ParseValidCSVFile) {
    std::string testFilename = "test_valid.csv";
    std::ofstream testFile(testFilename);
    
    testFile << "timestamp,RPM,SPEED,THROTTLE_POS,COOLANT_TEMP,FUEL_LEVEL,STYLE\n";
    testFile << "1640995200,2500,80,25.5,90.0,75.0,NORMAL\n";
    testFile << "1640995201,2600,82,26.0,91.0,74.5,AGGRESSIVE\n";
    testFile << "1640995202,1500,45,15.0,85.0,90.0,SLOW\n";
    testFile << "1640995203,3500,110,55.0,92.0,65.0,NORMAL\n";
    testFile << "1640995204,4200,130,75.0,96.0,50.0,AGGRESSIVE\n";
    testFile.close();
    
    int result = obj->parseCSVFile(testFilename);
    
    EXPECT_EQ(result, 5);
    EXPECT_EQ(obj->getRecordCount(), 5);
    
    OBDRecord firstRecord = obj->getOBDRecord(0);
    EXPECT_EQ(firstRecord.getRPM(), 2500);
    EXPECT_EQ(firstRecord.getSpeed(), 80);
    EXPECT_DOUBLE_EQ(firstRecord.getThrottlePos(), 25.5);
    EXPECT_DOUBLE_EQ(firstRecord.getCoolantTemp(), 90.0);
    EXPECT_DOUBLE_EQ(firstRecord.getFuelLevel(), 75.0);
    EXPECT_EQ(firstRecord.getStyle(), DrivingStyle::NORMAL);
    
    OBDRecord secondRecord = obj->getOBDRecord(1);
    EXPECT_EQ(secondRecord.getRPM(), 2600);
    EXPECT_EQ(secondRecord.getSpeed(), 82);
    EXPECT_EQ(secondRecord.getStyle(), DrivingStyle::AGGRESSIVE);
    
    OBDRecord thirdRecord = obj->getOBDRecord(2);
    EXPECT_EQ(thirdRecord.getRPM(), 1500);
    EXPECT_EQ(thirdRecord.getSpeed(), 45);
    EXPECT_EQ(thirdRecord.getStyle(), DrivingStyle::SLOW);
    
    std::remove(testFilename.c_str());
}

TEST_F(OBDParserTest, ParseFileWithInvalidLine) {
    std::string testFilename = "test_with_invalid.csv";
    std::ofstream testFile(testFilename);
    
    testFile << "timestamp,RPM,SPEED,THROTTLE_POS,COOLANT_TEMP,FUEL_LEVEL,STYLE\n";
    testFile << "1640995200,2500,80,25.5,90.0,75.0,NORMAL\n";
    testFile << "INVALID_LINE_WITHOUT_ENOUGH_FIELDS\n";  // Некорректная строка
    testFile << "1640995201,2600,82,26.0,91.0,74.5,AGGRESSIVE\n";
    testFile << "1640995202,1500,45,15.0,85.0,90.0\n";  // Недостаточно полей (нет STYLE)
    testFile << "1640995203,3500,110,55.0,92.0,65.0,NORMAL\n";
    testFile << "abc,def,ghi,jkl,mno,pqr,stu\n";  // Некорректные числа
    testFile << "1640995204,4200,130,75.0,96.0,50.0,AGGRESSIVE\n";
    testFile.close();
    
    int result = obj->parseCSVFile(testFilename);
    
    EXPECT_EQ(result, 5);
    EXPECT_EQ(obj->getRecordCount(), 5);
    
    OBDRecord firstRecord = obj->getOBDRecord(0);
    EXPECT_EQ(firstRecord.getRPM(), 2500);
    EXPECT_EQ(firstRecord.getSpeed(), 80);
    EXPECT_EQ(firstRecord.getStyle(), DrivingStyle::NORMAL);
    
    OBDRecord secondRecord = obj->getOBDRecord(1);
    EXPECT_EQ(secondRecord.getRPM(), 2600);
    EXPECT_EQ(secondRecord.getStyle(), DrivingStyle::AGGRESSIVE);
    
    OBDRecord thirdRecord = obj->getOBDRecord(2);
    EXPECT_EQ(thirdRecord.getRPM(), 3500);
    EXPECT_EQ(thirdRecord.getStyle(), DrivingStyle::NORMAL);
    
    OBDRecord defaultRecord = obj->getOBDRecord(3);
    EXPECT_EQ(defaultRecord.getTimestamp(), 0);
    EXPECT_EQ(defaultRecord.getRPM(), 0);
    EXPECT_EQ(defaultRecord.getSpeed(), 0);
    EXPECT_DOUBLE_EQ(defaultRecord.getThrottlePos(), 0.0);
    EXPECT_DOUBLE_EQ(defaultRecord.getCoolantTemp(), 0.0);
    EXPECT_DOUBLE_EQ(defaultRecord.getFuelLevel(), 0.0);
    EXPECT_EQ(defaultRecord.getStyle(), DrivingStyle::UNKNOWN);
    
    OBDRecord fourthRecord = obj->getOBDRecord(4);
    EXPECT_EQ(fourthRecord.getRPM(), 4200);
    EXPECT_EQ(fourthRecord.getStyle(), DrivingStyle::AGGRESSIVE);
    
    std::remove(testFilename.c_str());
}