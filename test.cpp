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
        cv::Mat imgl = cv::imread(left,  cv::IMREAD_GRAYSCALE);
        cv::Mat imgr = cv::imread(right, cv::IMREAD_GRAYSCALE);
        calib.process(imgl, imgr);

        while((char)cv::waitKey() != ' ');
    }

    return 0;
}

