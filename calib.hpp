
#ifndef DEF_LIBCV_CALIB
#define DEF_LIBCV_CALIB

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

namespace libcv
{
    class Compute;

    class CalibCam
    {
        friend Compute;
        public:
            CalibCam();
            CalibCam(const CalibCam&) = delete;
            ~CalibCam();

            /* Load the calibration datas from a file */
            bool load(const std::string& path);
            /* Save the calibration data to a file */
            bool save(const std::string& path) const;

            /* Calibrate the cameras with a list of pictures from the two cameras
             * cam1 and cam2 must have the same size
             */
            void start(int nx, int ny, float size, CvSize img);
            void end();
            bool compute(const cv::Mat& i1, const cv::Mat& i2, bool gray = false);

            void transform(cv::Mat& left, cv::Mat& right);

        private:
            int m_nx;
            int m_ny;
            float m_size;
            CvSize m_imgSize;
            bool m_display;

            std::vector<std::vector<cv::Point3f>> m_objectPoints;
            std::vector<std::vector<cv::Point2f>> m_imagePoints1;
            std::vector<std::vector<cv::Point2f>> m_imagePoints2;
            unsigned int m_success;

            cv::Mat CM1, CM2, D1, D2;
            cv::Mat R, T, E, F, Q;
            cv::Mat R1, R2, P1, P2;
            cv::Mat m_mx1, m_mx2, m_my1, m_my2;

            /* Internal functions */
            std::vector<cv::Point2f> computeOne(const cv::Mat& img, bool gr = false, const std::string& win = "");
    };
}

#endif

