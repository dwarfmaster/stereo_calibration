#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include "calib.hpp"

int main()
{
    std::ifstream ifs("list");
    if(!ifs)
        return 1;
    std::string line;
    libcv::CalibCam calib;
    calib.start(9, 6, 2.5, cvSize(640, 480));

    while(std::getline(ifs, line)) {
        std::istringstream iss(line);
        std::string left, right;
        iss >> left >> right;

        cv::Mat imgl  = cv::imread(left, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat imgr = cv::imread(right, CV_LOAD_IMAGE_GRAYSCALE);
        if(calib.compute(imgl, imgr, true))
            std::cout << "Computing pictures " << left << " and " << right << std::endl;
        else
            std::cout << "Couldn't compute pictures " << left << " and " << right << std::endl;

        while((char)cvWaitKey(0) != ' ');
    }

    std::cout << "Computing ..." << std::endl;
    calib.end();
    calib.save("save.xml");
    return 0;
}

