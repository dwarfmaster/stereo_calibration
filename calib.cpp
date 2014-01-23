
#include "calib.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

using namespace cv;

namespace libcv
{
    /* Lvl of gray per meter. 
     * TODO find value.
     */
    const float lvlpermeter = 1.0f;

    CalibCam::CalibCam()
        : m_display(true)
    {}

    CalibCam::~CalibCam()
    {
    }

    bool CalibCam::load(const std::string& path)
    {
        FileStorage fs(path, FileStorage::READ);
        fs["CM1"] >> CM1;
        fs["CM2"] >> CM2;
        fs["D1"]  >> D1;
        fs["D2"]  >> D2;
        fs["R"]   >> R;
        fs["T"]   >> T;
        fs["E"]   >> E;
        fs["F"]   >> F;
        fs["R1"]  >> R1;
        fs["R2"]  >> R2;
        fs["P1"]  >> P1;
        fs["P2"]  >> P2;
        fs["Q"]   >> Q;
        float w, h;
        fs["W"]   >> w;
        fs["H"]   >> h;
        m_imgSize = Size(w, h);
        fs["MX1"] >> m_mx1;
        fs["MX2"] >> m_mx2;
        fs["MY1"] >> m_my1;
        fs["MY2"] >> m_my2;

        FileStorage test("tmp.xml", FileStorage::WRITE);
        test << "MX1" << m_mx1;
        test << "MX2" << m_mx2;
        test << "MY1" << m_my1;
        test << "MY2" << m_my2;
        test.release();

        return true;
    }

    bool CalibCam::save(const std::string& path) const
    {
        FileStorage fs(path, FileStorage::WRITE);
        fs << "CM1" << CM1;
        fs << "CM2" << CM2;
        fs << "D1"  << D1;
        fs << "D2"  << D1;
        fs << "R"   << R;
        fs << "T"   << T;
        fs << "E"   << E;
        fs << "F"   << F;
        fs << "R1"  << R1;
        fs << "R2"  << R2;
        fs << "P1"  << P1;
        fs << "P2"  << P2;
        fs << "Q"   << Q;
        fs << "W"   << m_imgSize.width;
        fs << "H"   << m_imgSize.height;
        fs << "MX1" << m_mx1;
        fs << "MX2" << m_mx2;
        fs << "MY1" << m_my1;
        fs << "MY2" << m_my2;
        fs.release();
        return true;
    }

    void CalibCam::start(int nx, int ny, float size, CvSize img)
    {
        m_nx = nx;
        m_ny = ny;
        m_size = size;
        m_imgSize = img;
        m_success = 0;
        m_objectPoints.clear();
        m_imagePoints1.clear();
        m_imagePoints2.clear();
    }

    void CalibCam::end()
    {
        CM1 = Mat(3, 3, CV_64FC1);
        CM2 = Mat(3, 3, CV_64FC1);

        stereoCalibrate(m_objectPoints, m_imagePoints1, m_imagePoints2,
                CM1, D1, CM2, D2, m_imgSize, R, T, E, F,
                cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),
                CV_CALIB_SAME_FOCAL_LENGTH | CV_CALIB_ZERO_TANGENT_DIST);

        stereoRectify(CM1, D1, CM2, D2, m_imgSize, R, T, R1, R2, P1, P2, Q);

        initUndistortRectifyMap(CM1, D1, R1, P1, m_imgSize, CV_32FC1, m_mx1, m_my1);
        initUndistortRectifyMap(CM2, D2, R2, P2, m_imgSize, CV_32FC1, m_mx2, m_my2);
    }

    bool CalibCam::compute(const cv::Mat& i1, const cv::Mat& i2, bool gray)
    {
        std::vector<Point2f> left  = computeOne(i1, gray, "Left");
        std::vector<Point2f> right = computeOne(i2, gray, "Right");

        if(!left.empty() && !right.empty()) {
            m_imagePoints1.push_back(left);
            m_imagePoints2.push_back(right);

            std::vector<Point3f> obj;
            obj.reserve(m_nx * m_ny);
            for(int i = 0; i < (m_nx*m_ny); ++i)
                obj.push_back(Point3f(i/m_nx, i%m_nx, 0.0));
            m_objectPoints.push_back(obj);
            ++m_success;
            return true;
        }
        return false;
    }

    std::vector<Point2f> CalibCam::computeOne(const cv::Mat& img, bool gr, const std::string& win)
    {
        Mat imgmat(img);
        Mat gray;
        if(gr)
            gray = imgmat;
        else
            cvtColor(imgmat, gray, CV_BGR2GRAY);

        std::vector<Point2f> corners;
        bool found = findChessboardCorners(gray, Size(m_nx, m_ny), corners, 
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

        if(m_display) {
            auto dup = corners;
            cornerSubPix(gray, dup, Size(11, 11), Size(-1, -1),
                    TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
            cvtColor(gray, imgmat, CV_GRAY2BGR);
            drawChessboardCorners(imgmat, Size(m_nx, m_ny), dup, found);
            imshow(win, imgmat);
        }

        if(found)
            return corners;
        else
            return std::vector<Point2f>();
    }

    Mat global;
    void onMouse(int event, int x, int y, int, void*)
    {
        if(event != CV_EVENT_LBUTTONDOWN)
            return;
        std::cout << "(" << x << ";" << y << ") = " << global.at<float>(x,y) << std::endl;
    }

    void CalibCam::process(const Mat& left, const Mat& right)
    {
        Mat tmpl, tmpr;
        remap(left,  tmpl, m_mx1, m_my1, INTER_LINEAR, BORDER_CONSTANT, Scalar());
        remap(right, tmpr, m_mx2, m_my2, INTER_LINEAR, BORDER_CONSTANT, Scalar());

        StereoBM bm(CV_STEREO_BM_BASIC, 64, 65);
        bm.state->preFilterSize     = 5;
        bm.state->preFilterCap      = 10;
        bm.state->minDisparity      = 0;
        bm.state->textureThreshold  = 0;
        bm.state->uniquenessRatio   = 5;
        bm.state->speckleWindowSize = 0;
        bm.state->speckleRange      = 0;

        Mat disp, vdisp;
        bm(tmpl, tmpr, disp, CV_32F);
        normalize(disp, vdisp, 0, 255, NORM_MINMAX, CV_8UC1);
        imshow("Normalized Disparity", vdisp);
        // disp.convertTo(disp, CV_8U, 255, 0);
        global = disp;
        imshow("Disparity", disp);
        setMouseCallback("Disparity", onMouse, NULL);

        computeDists(vdisp);
    }
            
    void CalibCam::computeDists(const cv::Mat& disp)
    {
        m_dists = Mat::zeros(3, 5, CV_32F);
        for(int x = 0; x < 5; ++x) {
            for(int y = 0; y < 3; ++y) {
                Mat sub = disp(Range(y*160, y*160+159),
                        Range(x*128, x*128+127));
                histoFill(y, x, sub);
            }
        }

        /* Print m_dists for debug */
        for(int y = 0; y < 3; ++y) {
            std::cout << "| ";
            for(int x = 0; x < 5; ++x)
                std::cout << m_dists.at<float>(y,x) << " ";
            std::cout << "|" << std::endl;
        }
        std::cout << std::endl;
    }

    void CalibCam::histoFill(int y, int x, const cv::Mat& part)
    {
        Mat hist;
        float range[] = {0, 256};
        const float* histRange = { range };
        int histSize = 256;
        calcHist(&part, 1, 0, Mat(), hist, 1, &histSize, &histRange, true, false);

        /* Cumulate hist. */
        for(int i = 1; i < 256; ++i)
            hist.at<float>(i) += hist.at<float>(i-1);
        float lvl = 0.95 * hist.at<float>(255); /* Allow a 5 percent error in max. */

        int i;
        for(i = 0; hist.at<float>(i) < lvl; ++i);
        m_dists.at<float>(y, x) = (float)i * lvlpermeter;
    }

}


