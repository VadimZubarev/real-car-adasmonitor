#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <string>

#include <opencv2/core.hpp>

#include "obd_parser.h"

class Dashboard {
public:
    Dashboard(int panelWidth = 640, int panelHeight = 480);

    void draw(cv::Mat& frame, const OBDRecord& record) const;

private:
    int panelWidth_;
    int panelHeight_;

    static double clampValue(double value, double minValue, double maxValue);
    static std::string styleToString(DrivingStyle style);
    static cv::Scalar styleColor(DrivingStyle style);

    void drawGauge(cv::Mat& image,
                   const cv::Point& center,
                   int radius,
                   double value,
                   double minValue,
                   double maxValue,
                   const std::string& title,
                   const std::string& unit,
                   double warningThreshold) const;

    void drawLinearGauge(cv::Mat& image,
                         const cv::Rect& rect,
                         double value,
                         double minValue,
                         double maxValue,
                         const std::string& label,
                         const cv::Scalar& normalColor,
                         const cv::Scalar& warningColor,
                         bool warningHigh,
                         double warningThreshold) const;
};

#endif  // DASHBOARD_H
