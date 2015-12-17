#ifndef __JRPANO_PIPELINE_H__
#define __JRPANO_PIPELINE_H__

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/stitching/warpers.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"

#include "exceptions.h"

class Pipeline {
private:
    size_t num_imgs_;
    size_t num_homos_;
    double work_area_ = 0.6;
    double seam_area_ = 10000;
    double hess_thresh_ = 1500;
    float match_conf_ = 0.3f;
    int blend_bands_ = 3;
    int warp_mode_ = 0;

    cv::Mat result_;
    std::vector<cv::Mat> imgs_;
    std::vector<std::string> imgpaths_;
    std::vector<std::vector<cv::KeyPoint>> keypoints_;
    std::vector<std::vector<cv::DMatch>> matches_;  // matches between adjacent imgs
    std::vector<cv::Mat> homos_;  // homographies between adjancent imgs
    std::vector<int> num_inliers_;
    std::vector<cv::detail::CameraParams> cameras_;
    cv::Ptr<cv::WarperCreator> creator_ = new cv::SphericalWarper();
    std::vector<cv::Mat> masks_warped_;
    std::vector<std::vector<cv::Point>> contours_;

public:
    Pipeline();
    void ReadImages(const std::vector<std::string>& imgpaths);
    void DetectKeypoints();
    void FindMatchesAndHomos();
    void EstimateFocal();
    void CalculateRotation();
    void FindSeams();
    void WarpAndComposite();
    void DrawMatches();
    void ShowResult();
    void WriteResult(std::string path);
    void ShowOriginalPart();

    // getter
    std::vector<cv::Mat> homos();
    std::vector<cv::detail::CameraParams> cameras();
    std::vector<std::vector<cv::KeyPoint>> keypoints();
    std::vector<cv::Mat> imgs();
    std::vector<std::vector<cv::DMatch>> matches();
    double hess_thresh();
    float match_conf();
    double seam_area();
    double work_area();
    int blend_bands();
    int warp_mode();
    cv::Mat result();

    // setter
    void set_hess_thresh(double hess_thresh);
    void set_match_conf(float match_conf);
    void set_seam_area(double seam_area);
    void set_work_area(double work_area);
    void set_blend_bands(int blend_bands);
    void set_warp_mode(int warp_mode);
};

#endif // #ifndef __JRPANO_PIPELINE_H__