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

        IplImage* imgl  = cvLoadImage(left.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
        IplImage* imgr = cvLoadImage(right.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
        if(calib.compute(imgl, imgr, true))
            std::cout << "Computing pictures " << left << " and " << right << std::endl;
        else
            std::cout << "Couldn't compute pictures " << left << " and " << right << std::endl;

        cvRelease((void**)&imgl);
        cvRelease((void**)&imgr);
        while((char)cvWaitKey(0) != ' ');
    }

    std::cout << "Computing ..." << std::endl;
    calib.end();
    calib.save("save.xml");
    return 0;
}

