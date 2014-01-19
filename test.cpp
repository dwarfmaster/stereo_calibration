#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "calib.hpp"

int main()
{
    std::ifstream ifs("list_test");
    std::string line;
    libcv::CalibCam calib;
    calib.load("save.xml");

    while(std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string left, right;
        iss >> left >> right;

        std::cout << "Remapping " << left << " and " << right << std::endl;
        cv::Mat imgml = cv::imread(left,  cv::IMREAD_GRAYSCALE);
        cv::Mat imgmr = cv::imread(right, cv::IMREAD_GRAYSCALE);
        calib.transform(imgml, imgmr);

        cv::Mat imgpl, imgpr;
        cv::cvtColor(imgml, imgpl, CV_GRAY2BGR);
        cv::cvtColor(imgmr, imgpr, CV_GRAY2BGR);
        for(size_t i = 0; i < 480; i += 16) {
            cv::line(imgpl, cv::Point(0, i),
                    cv::Point(640, i), cv::Scalar(0, 255, 0));
            cv::line(imgpr, cv::Point(0, i),
                    cv::Point(640, i), cv::Scalar(0, 255, 0));
        }
        cv::imshow("Left",  imgpl);
        cv::imshow("Right", imgpr);
        
        // Saving
        std::string path = "trans/" + left;
        cv::imwrite(path, imgml);
        path = "trans/" + right;
        cv::imwrite(path, imgmr);

        while((char)cv::waitKey() != ' ');
    }

    return 0;
}

