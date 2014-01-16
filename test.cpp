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
        IplImage* imgl  = cvLoadImage(left.c_str());
        IplImage* imgr = cvLoadImage(right.c_str());
        cv::Mat imgml(imgl);
        cv::Mat imgmr(imgr);
        cv::cvtColor(imgml, imgml, CV_BGR2GRAY);
        cv::cvtColor(imgmr, imgmr, CV_BGR2GRAY);
        calib.transform(imgml, imgmr);

        cv::cvtColor(imgml, imgml, CV_GRAY2BGR);
        cv::cvtColor(imgmr, imgmr, CV_GRAY2BGR);
        IplImage imgpl = imgml;
        IplImage imgpr = imgmr;
        for(size_t i = 0; i < 480; i += 16) {
            cvLine(&imgpl, cv::Point(0, i),
                    cv::Point(640, i), CV_RGB(0, 255, 0));
            cvLine(&imgpr, cv::Point(0, i),
                    cv::Point(640, i), CV_RGB(0, 255, 0));
        }
        cvShowImage("Left", &imgpl);
        cvShowImage("Right", &imgpr);
        
        // Saving
        std::string path = "trans/" + left;
        cvSaveImage(path.c_str(), (void*)&imgpl);
        path = "trans/" + right;
        cvSaveImage(path.c_str(), (void*)&imgpr);

        cvRelease((void**)&imgl);
        cvRelease((void**)&imgr);
        while((char)cvWaitKey(0) != ' ');
    }

    return 0;
}

