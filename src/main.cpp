#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "dashboard.h"

int main() {
    cv::Mat frame(480, 1280, CV_8UC3, cv::Scalar(40, 40, 40));
    cv::rectangle(frame, cv::Rect(640, 0, 640, 480), cv::Scalar(60, 55, 45), cv::FILLED);
    cv::putText(frame,
                "Static camera frame area",
                cv::Point(790, 240),
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                cv::Scalar(220, 220, 220),
                2,
                cv::LINE_AA);

    OBDRecord testRecord;
    testRecord.setSpeed(102);
    testRecord.setRPM(4700);
    testRecord.setCoolantTemp(107.0);
    testRecord.setFuelLevel(12.0);
    testRecord.setThrottlePos(68.0);
    testRecord.setStyle(DrivingStyle::AGGRESSIVE);

    Dashboard dashboard(640, 480);
    dashboard.draw(frame, testRecord);

    cv::imshow("Dashboard test", frame);
    cv::waitKey(0);
    return 0;
}