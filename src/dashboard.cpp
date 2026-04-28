#include "dashboard.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include <opencv2/imgproc.hpp>

namespace {
constexpr double kGaugeStartDeg = 135.0;
constexpr double kGaugeSweepDeg = 270.0;
}  // namespace

Dashboard::Dashboard(int panelWidth, int panelHeight)
    : panelWidth_(panelWidth), panelHeight_(panelHeight) {}

double Dashboard::clampValue(double value, double minValue, double maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

std::string Dashboard::styleToString(DrivingStyle style) {
    switch (style) {
        case DrivingStyle::NORMAL:
            return "NORMAL";
        case DrivingStyle::AGGRESSIVE:
            return "AGGRESSIVE";
        case DrivingStyle::SLOW:
            return "SLOW";
        default:
            return "UNKNOWN";
    }
}

cv::Scalar Dashboard::styleColor(DrivingStyle style) {
    switch (style) {
        case DrivingStyle::NORMAL:
            return cv::Scalar(0, 220, 0);
        case DrivingStyle::AGGRESSIVE:
            return cv::Scalar(0, 0, 255);
        case DrivingStyle::SLOW:
            return cv::Scalar(255, 255, 0);
        default:
            return cv::Scalar(220, 220, 220);
    }
}

void Dashboard::drawGauge(cv::Mat& image,
                          const cv::Point& center,
                          int radius,
                          double value,
                          double minValue,
                          double maxValue,
                          const std::string& title,
                          const std::string& unit,
                          double warningThreshold) const {
    cv::circle(image, center, radius, cv::Scalar(80, 80, 80), 2, cv::LINE_AA);
    cv::circle(image, center, radius - 15, cv::Scalar(60, 60, 60), 1, cv::LINE_AA);

    for (int i = 0; i <= 10; ++i) {
        const double t = static_cast<double>(i) / 10.0;
        const double angleDeg = kGaugeStartDeg + t * kGaugeSweepDeg;
        const double angleRad = angleDeg * CV_PI / 180.0;
        const cv::Point p1(center.x + static_cast<int>((radius - 5) * std::cos(angleRad)),
                           center.y - static_cast<int>((radius - 5) * std::sin(angleRad)));
        const cv::Point p2(center.x + static_cast<int>((radius - 18) * std::cos(angleRad)),
                           center.y - static_cast<int>((radius - 18) * std::sin(angleRad)));
        cv::line(image, p1, p2, cv::Scalar(190, 190, 190), 2, cv::LINE_AA);
    }

    const double clamped = clampValue(value, minValue, maxValue);
    const double ratio = (clamped - minValue) / (maxValue - minValue);
    const double endDeg = kGaugeStartDeg + ratio * kGaugeSweepDeg;

    const cv::Scalar arcColor = (value > warningThreshold) ? cv::Scalar(0, 0, 255)
                                                            : cv::Scalar(0, 210, 0);
    cv::ellipse(image,
                center,
                cv::Size(radius - 10, radius - 10),
                0.0,
                kGaugeStartDeg,
                endDeg,
                arcColor,
                8,
                cv::LINE_AA);

    const double needleRad = endDeg * CV_PI / 180.0;
    const cv::Point needle(center.x + static_cast<int>((radius - 25) * std::cos(needleRad)),
                           center.y - static_cast<int>((radius - 25) * std::sin(needleRad)));
    cv::line(image, center, needle, cv::Scalar(220, 220, 220), 3, cv::LINE_AA);
    cv::circle(image, center, 6, cv::Scalar(220, 220, 220), cv::FILLED, cv::LINE_AA);

    cv::putText(image,
                title,
                cv::Point(center.x - radius + 10, center.y - radius - 10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.6,
                cv::Scalar(220, 220, 220),
                2,
                cv::LINE_AA);

    std::ostringstream valueText;
    valueText << std::fixed << std::setprecision(0) << clamped << " " << unit;
    cv::putText(image,
                valueText.str(),
                cv::Point(center.x - radius / 2, center.y + radius / 2),
                cv::FONT_HERSHEY_SIMPLEX,
                0.45,
                cv::Scalar(250, 250, 250),
                2,
                cv::LINE_AA);
}

void Dashboard::drawLinearGauge(cv::Mat& image,
                                const cv::Rect& rect,
                                double value,
                                double minValue,
                                double maxValue,
                                const std::string& label,
                                const cv::Scalar& normalColor,
                                const cv::Scalar& warningColor,
                                bool warningHigh,
                                double warningThreshold) const {
    cv::rectangle(image, rect, cv::Scalar(70, 70, 70), 2, cv::LINE_AA);
    cv::rectangle(image, rect, cv::Scalar(30, 30, 30), cv::FILLED, cv::LINE_AA);

    const double clamped = clampValue(value, minValue, maxValue);
    const double ratio = (clamped - minValue) / (maxValue - minValue);
    const int fillWidth = static_cast<int>((rect.width - 4) * ratio);

    const bool warning = warningHigh ? (value > warningThreshold) : (value < warningThreshold);
    const cv::Scalar fillColor = warning ? warningColor : normalColor;

    cv::Rect fillRect(rect.x + 2, rect.y + 2, std::max(0, fillWidth), rect.height - 4);
    if (fillRect.width > 0) {
        cv::rectangle(image, fillRect, fillColor, cv::FILLED, cv::LINE_AA);
    }

    std::ostringstream text;
    text << label << ": " << std::fixed << std::setprecision(0) << clamped;
    cv::putText(image,
                text.str(),
                cv::Point(rect.x, rect.y - 8),
                cv::FONT_HERSHEY_SIMPLEX,
                0.55,
                cv::Scalar(230, 230, 230),
                2,
                cv::LINE_AA);
}

void Dashboard::draw(cv::Mat& frame, const OBDRecord& record) const {
    if (frame.empty()) {
        return;
    }

    const int overlayWidth = std::min(panelWidth_, frame.cols);
    const int overlayHeight = std::min(panelHeight_, frame.rows);
    if (overlayWidth <= 0 || overlayHeight <= 0) {
        return;
    }

    cv::Rect panelRect(0, 0, overlayWidth, overlayHeight);
    // cv::Mat overlay = frame.clone();
    cv::Mat panel = frame(panelRect);

    cv::Mat overlay(panel.size(), panel.type(), cv::Scalar(20, 20, 20));
    cv::addWeighted(overlay, 0.6, panel, 0.4, 0.0, panel);

    // cv::rectangle(overlay, panelRect, cv::Scalar(20, 20, 20), cv::FILLED);
    // cv::addWeighted(overlay, 0.58, frame, 0.42, 0.0, frame);

    const cv::Point speedCenter(overlayWidth / 4, overlayHeight / 4);
    const cv::Point rpmCenter(overlayWidth / 4, 2 * overlayHeight / 3);
    const int radius = std::min(115, overlayHeight / 6);

    drawGauge(frame,
              speedCenter,
              radius,
              record.getSpeed(),
              0.0,
              140.0,
              "SPEED",
              "km/h",
              90.0);

    drawGauge(frame,
              rpmCenter,
              radius,
              record.getRPM(),
              0.0,
              6000.0,
              "RPM",
              "rpm",
              4500.0);

    const int x = overlayWidth / 2 + 20;
    const int width = overlayWidth / 2 - 40;
    const int barHeight = 26;
    const int y0 = 130;
    const int gap = 72;

    drawLinearGauge(frame,
                    cv::Rect(x, y0, width, barHeight),
                    record.getCoolantTemp(),
                    0.0,
                    120.0,
                    "Coolant (C)",
                    cv::Scalar(0, 200, 0),
                    cv::Scalar(0, 0, 255),
                    true,
                    100.0);

    drawLinearGauge(frame,
                    cv::Rect(x, y0 + gap, width, barHeight),
                    record.getFuelLevel(),
                    0.0,
                    100.0,
                    "Fuel (%)",
                    cv::Scalar(0, 190, 255),
                    cv::Scalar(0, 0, 255),
                    false,
                    15.0);

    drawLinearGauge(frame,
                    cv::Rect(x, y0 + 2 * gap, width, barHeight),
                    record.getThrottlePos(),
                    0.0,
                    100.0,
                    "Throttle (%)",
                    cv::Scalar(255, 180, 0),
                    cv::Scalar(0, 0, 255),
                    true,
                    90.0);

    cv::putText(frame,
                "Driving style:",
                cv::Point(x, y0 + 3 * gap + 10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.62,
                cv::Scalar(230, 230, 230),
                2,
                cv::LINE_AA);
    cv::putText(frame,
                styleToString(record.getStyle()),
                cv::Point(x, y0 + 3 * gap + 40),
                cv::FONT_HERSHEY_SIMPLEX,
                0.9,
                styleColor(record.getStyle()),
                2,
                cv::LINE_AA);

    int warningY = y0 + 3 * gap + 90;
    if (record.getCoolantTemp() > 100.0) {
        cv::putText(frame,
                    "WARNING: coolant temperature high",
                    cv::Point(20, warningY),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.62,
                    cv::Scalar(0, 0, 255),
                    2,
                    cv::LINE_AA);
        warningY += 30;
    }
    if (record.getFuelLevel() < 15.0) {
        cv::putText(frame,
                    "WARNING: low fuel level",
                    cv::Point(20, warningY),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.62,
                    cv::Scalar(0, 0, 255),
                    2,
                    cv::LINE_AA);
    }
}
