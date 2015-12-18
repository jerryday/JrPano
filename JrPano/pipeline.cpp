#include "stdafx.h"
#include "pipeline.h"


Pipeline::Pipeline() {
    cv::initModule_nonfree();
}


void Pipeline::ReadImages(const std::vector<std::string>& imgpaths) {
    imgs_.clear();

    for (auto& imgpath : imgpaths) {
        cv::Mat img = cv::imread(imgpath);
        if (!img.empty()) {
            imgs_.push_back(img);
        }
    }
    num_imgs_ = imgs_.size();
    num_homos_ = num_imgs_ - 1;
    imgpaths_ = imgpaths;

    if (num_imgs_ < 2) {
        throw JrTooFewImages("Error: Need at least 2 images");
    }

    float work_scale = std::min(1.0f, (float)sqrt(work_area_ * 1e6 / imgs_[0].size().area()));
    for (auto& img : imgs_) {
        cv::resize(img, img, cv::Size(), work_scale, work_scale);
    }
}


void Pipeline::DetectKeypoints() {
    cv::Ptr<cv::FeatureDetector> detector = new cv::SurfFeatureDetector(hess_thresh_,3,4);
    detector->detect(imgs_, keypoints_);
}


void Pipeline::FindMatchesAndHomos() {
    if (match_conf_ < 0 || match_conf_ > 1) {
        throw JrInvalidArgument("Error: Match confidence should be in range [0, 1]");
    }

    cv::Ptr<cv::DescriptorExtractor> extractor = new cv::SurfDescriptorExtractor();
    extractor->set("nOctaves", 3);
    extractor->set("nOctaveLayers", 4);
    std::vector<cv::Mat> descriptors(keypoints_.size());
    for (size_t i = 0; i < keypoints_.size(); i++) {
        extractor->compute(imgs_[i], keypoints_[i], descriptors[i]);
    }

    cv::Ptr<cv::DescriptorMatcher> matcher = new cv::FlannBasedMatcher();
    matches_ = std::vector<std::vector<cv::DMatch>>(num_homos_);
    homos_ = std::vector<cv::Mat>(num_homos_);
    num_inliers_ = std::vector<int>(num_homos_);
    for (size_t i = 0; i < num_homos_; i++) {
        // using best of two algorithm to find good matches
        std::vector<std::vector<cv::DMatch>> knnmatches;
        matcher->knnMatch(descriptors[i], descriptors[i + 1], knnmatches, 2);

        for (auto m : knnmatches) {
            if (m.size() < 2)
                continue;

            if (m[0].distance < (1.0f - match_conf_) * m[1].distance) {
                matches_[i].push_back(m[0]);
            }
        }
        //cv::Mat imgm;
        //cv::drawMatches(imgs_[i], keypoints_[i], imgs_[i+1], keypoints_[i+1], matches_[i], imgm);
        //cv::namedWindow("qwe");
        //cv::imshow("qwe", imgm);
        //cv::waitKey(0);
        //cv::destroyAllWindows();
        if (matches_[i].size() <= 6) {
            throw JrUnmatchedPairs("Error: Can't match between [" + imgpaths_[i] + ", " + imgpaths_[i + 1] + "], try to descrease match confidence");
        }

        std::vector<cv::Point2f> srcpoints, dstpoints;
        for (auto m : matches_[i]) {
            cv::Point2f p = keypoints_[i][m.queryIdx].pt;
            p.x -= imgs_[i].size().width * 0.5f;
            p.y -= imgs_[i].size().height * 0.5f;
            srcpoints.push_back(p);

            p = keypoints_[i + 1][m.trainIdx].pt;
            p.x -= imgs_[i + 1].size().width * 0.5f;
            p.y -= imgs_[i + 1].size().height * 0.5f;
            dstpoints.push_back(p);
        }

        std::vector<uchar> inlier_masks;
        homos_[i] = cv::findHomography(srcpoints, dstpoints, inlier_masks, CV_RANSAC);

        int num_inlier = 0;
        for (uchar inlier_mask : inlier_masks) {
            if (inlier_mask)
                num_inlier++;
        }
        num_inliers_[i] = num_inlier;

        //if (num_inliers_[i] / (8 + 0.3 * matches_[i].size()) < 0.8) {
        //    throw JrUnmatchedPairs("Error: Can't match between [" + imgpaths_[i] + ", " + imgpaths_[i + 1] + "]");
        //}
    }
}


void Pipeline::EstimateFocal() {
    double f0, f1;
    bool f0ok, f1ok;
    std::vector<double> focals;
    for (cv::Mat homo : homos_) {
        cv::detail::focalsFromHomography(homo, f0, f1, f0ok, f1ok);
        if (f0ok && f1ok)
            focals.push_back(sqrt(f0 * f1));
    }

    double result;
    if (focals.size() + 1 >= num_imgs_) {
        std::sort(focals.begin(), focals.end());
        if (focals.size() % 2 == 1) {
            result = focals[focals.size() / 2];
        } else {
            result = (focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) / 2;
        }
    } else {
        double focals_sum = 0;
        for (size_t i = 0; i < num_imgs_; i++) {
            focals_sum += imgs_[i].size().width + imgs_[i].size().height;
        }
        result = focals_sum / num_imgs_;
    }

    cameras_ = std::vector<cv::detail::CameraParams>(num_imgs_);
    for (size_t i = 0; i < num_imgs_; i++) {
        cameras_[i].focal = result;
    }
}

void Pipeline::CalculateRotation() {
    size_t medianid = num_imgs_ / 2;
    if (num_imgs_ % 2 == 0) {
        medianid--;
    }

    for (size_t i = medianid; i < num_imgs_ - 1; i++) {
        cv::Mat_<double> K_from = cv::Mat::eye(3, 3, CV_64F);
        K_from(0, 0) = cameras_[i].focal;
        K_from(1, 1) = cameras_[i].focal * cameras_[i].aspect;
        K_from(0, 2) = cameras_[i].ppx;
        K_from(1, 2) = cameras_[i].ppy;

        cv::Mat_<double> K_to = cv::Mat::eye(3, 3, CV_64F);
        K_to(0, 0) = cameras_[i + 1].focal;
        K_to(1, 1) = cameras_[i + 1].focal * cameras_[i + 1].aspect;
        K_to(0, 2) = cameras_[i + 1].ppx;
        K_to(1, 2) = cameras_[i + 1].ppy;

        cv::Mat R = K_from.inv() * homos_[i].inv() * K_to;
        cameras_[i + 1].R = cameras_[i].R * R;
    }

    for (size_t i = medianid; i > 0; i--) {
        cv::Mat_<double> K_from = cv::Mat::eye(3, 3, CV_64F);
        K_from(0, 0) = cameras_[i].focal;
        K_from(1, 1) = cameras_[i].focal * cameras_[i].aspect;
        K_from(0, 2) = cameras_[i].ppx;
        K_from(1, 2) = cameras_[i].ppy;

        cv::Mat_<double> K_to = cv::Mat::eye(3, 3, CV_64F);
        K_to(0, 0) = cameras_[i - 1].focal;
        K_to(1, 1) = cameras_[i - 1].focal * cameras_[i - 1].aspect;
        K_to(0, 2) = cameras_[i - 1].ppx;
        K_to(1, 2) = cameras_[i - 1].ppy;

        cv::Mat R = K_from.inv() * homos_[i - 1] * K_to;
        cameras_[i - 1].R = cameras_[i].R * R;
    }

    for (size_t i = 0; i < num_imgs_; i++) {
        cameras_[i].ppx += 0.5 * imgs_[i].size().width;
        cameras_[i].ppy += 0.5 * imgs_[i].size().height;
        cameras_[i].R.convertTo(cameras_[i].R, CV_32F);
        // std::cout << cameras_[i].R << std::endl;
    }
}


void Pipeline::FindSeams() {
    float seam_scale = std::min(1.0f, (float) sqrt(seam_area_ / imgs_[0].size().area()));
    cv::Ptr<cv::detail::RotationWarper> warper = creator_->create((float)cameras_[0].focal * seam_scale);

    std::vector<cv::Mat> imgs_warped_f(num_imgs_);
    std::vector<cv::Point> corners(num_imgs_);
    masks_warped_ = std::vector<cv::Mat>(num_imgs_);

    // shrink the images to accelerate seam finding
    std::vector<cv::Mat> imgs2;
    for (auto& img : imgs_) {
        cv::Mat img_resized;
        cv::resize(img, img_resized, cv::Size(), seam_scale, seam_scale);
        imgs2.push_back(img_resized.clone());
    }

    for (size_t i = 0; i < num_imgs_; i++) {
        cv::Mat_<float> K = cameras_[i].K();
        K(0, 0) *= seam_scale; K(0, 2) *= seam_scale;
        K(1, 1) *= seam_scale; K(1, 2) *= seam_scale;

        corners[i] = warper->warp(imgs2[i], K, cameras_[i].R, cv::INTER_LINEAR, cv::BORDER_REFLECT, imgs_warped_f[i]);

        cv::Mat mask(imgs2[i].size(), CV_8U, cv::Scalar::all(255));
        warper->warp(mask, K, cameras_[i].R, cv::INTER_LINEAR, cv::BORDER_CONSTANT, masks_warped_[i]);
    }

    for (auto& imgf : imgs_warped_f) {
        imgf.convertTo(imgf, CV_32F);
    }

    cv::Ptr<cv::detail::SeamFinder> finder = new cv::detail::GraphCutSeamFinder(cv::detail::GraphCutSeamFinderBase::COST_COLOR);
    finder->find(imgs_warped_f, corners, masks_warped_);
}


void Pipeline::WarpAndComposite() {
    std::vector<cv::Point> corners(num_imgs_);
    std::vector<cv::Size> wsizes(num_imgs_);
    cv::Ptr<cv::detail::RotationWarper> warper = creator_->create((float)cameras_[0].focal);
    for (size_t i = 0; i < num_imgs_; i++) {
        cv::Mat_<float> K = cameras_[i].K();
        cv::Rect roi = warper->warpRoi(imgs_[i].size(), K, cameras_[i].R);
        corners[i] = roi.tl();
        wsizes[i] = roi.size();
    }

    cv::Ptr<cv::detail::Blender> blender;
    if (blend_bands_ < 2) {
        blender = cv::detail::Blender::createDefault(cv::detail::Blender::NO);
    } else {
        blender = new cv::detail::MultiBandBlender(0, blend_bands_);
    }
    blender->prepare(corners, wsizes);
    cv::Point globaltl = cv::detail::resultRoi(corners, wsizes).tl();
    contours_.clear();
    
    cv::Mat img_warped, img_warped_s, mask_warped, dilate_mask, seam_mask;
    for (size_t i = 0; i < num_imgs_; i++) {
        cv::Mat_<float> K = cameras_[i].K();
        warper->warp(imgs_[i], K, cameras_[i].R, cv::INTER_LINEAR, cv::BORDER_REFLECT, img_warped);
        img_warped.convertTo(img_warped_s, CV_16S);

        cv::Mat mask(imgs_[i].size(), CV_8U, cv::Scalar::all(255));
        warper->warp(mask, K, cameras_[i].R, cv::INTER_LINEAR, cv::BORDER_CONSTANT, mask_warped);

        cv::dilate(masks_warped_[i], dilate_mask, cv::Mat());
        cv::resize(dilate_mask, seam_mask, mask_warped.size());
        mask_warped &= seam_mask;
        masks_warped_[i] = mask_warped;

        blender->feed(img_warped_s, mask_warped, corners[i]);

        cv::Point offset(corners[i].x - globaltl.x, corners[i].y - globaltl.y);
        std::vector<std::vector<cv::Point>> cts;
        cv::findContours(mask_warped, cts, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE, offset);
        contours_.insert(contours_.end(), cts.begin(), cts.end());
    }

    cv::Mat result, result_mask;
    blender->blend(result, result_mask);
    result.convertTo(result_, CV_8U);
}


void Pipeline::DrawMatches() {
    if (imgs_.empty() || keypoints_.empty() || matches_.empty())
        return;

    for (size_t i = 0; i < num_homos_; i++) {
        cv::Mat imgmatch;
        cv::drawMatches(imgs_[i], keypoints_[i], imgs_[i+1], keypoints_[i+1], matches_[i], imgmatch);
        std::string windowname = imgpaths_[i] + ", " + imgpaths_[i + 1];
        cv::namedWindow(windowname);
        cv::imshow(windowname, imgmatch);
    }
    cv::waitKey(0);
    cv::destroyAllWindows();
}


void Pipeline::ShowResult() {
    if (result_.empty())
        return;

    std::string windowname = "result.jpg";
    cv::namedWindow(windowname);
    cv::imshow(windowname, result_);
    cv::waitKey(0);
    cv::destroyWindow(windowname);
}


void Pipeline::ShowOriginalPart() {
    if (result_.empty())
        return;

    cv::Mat result = result_.clone();
    cv::RNG rng;

    for (size_t i = 0; i < contours_.size(); i++) {
        cv::Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        cv::drawContours(result, contours_, i, color, 2);
    }

    std::string windowname = "result_original_part.jpg";
    cv::namedWindow(windowname);
    cv::imshow(windowname, result);
    cv::waitKey(0);
    cv::destroyWindow(windowname);
}


void Pipeline::WriteResult(std::string path) {
    if (result_.empty())
        return;

    cv::imwrite(path, result_);
}


std::vector<cv::Mat> Pipeline::homos() {
    return homos_;
}

std::vector<cv::detail::CameraParams> Pipeline::cameras() {
    return cameras_;
}

std::vector<std::vector<cv::KeyPoint>> Pipeline::keypoints() {
    return keypoints_;
}

std::vector<cv::Mat> Pipeline::imgs() {
    return imgs_;
}

std::vector<std::vector<cv::DMatch>> Pipeline::matches() {
    return matches_;
}

double Pipeline::hess_thresh() {
    return hess_thresh_;
}

double Pipeline::seam_area() {
    return seam_area_;
}

double Pipeline::work_area() {
    return work_area_;
}

float Pipeline::match_conf() {
    return match_conf_;
}

int Pipeline::blend_bands() {
    return blend_bands_;
}

int Pipeline::warp_mode() {
    return warp_mode_;
}

cv::Mat Pipeline::result() {
    return result_;
}


void Pipeline::set_hess_thresh(double hess_thresh) {
    hess_thresh_ = hess_thresh;
}

void Pipeline::set_seam_area(double seam_area) {
    seam_area_ = seam_area;
}

void Pipeline::set_work_area(double work_area) {
    work_area_ = work_area;
}

void Pipeline::set_match_conf(float match_conf) {
    match_conf_ = match_conf;
}

void Pipeline::set_blend_bands(int blend_bands) {
    blend_bands_ = blend_bands;
}

void Pipeline::set_warp_mode(int warp_mode) {
    warp_mode_ = warp_mode;
    if (1 == warp_mode) {
        creator_ = new cv::CylindricalWarper();
    } else if (2 == warp_mode) {
        creator_ = new cv::PlaneWarper();
    } else {
        warp_mode_ = 0;
        creator_ = new cv::SphericalWarper();
    }
}