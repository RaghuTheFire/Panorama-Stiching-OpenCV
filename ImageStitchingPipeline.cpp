/******************************************************************************************************************************************
Image Stitching Pipeline from images directory using Keypoint detection, Feature matching, Homography, RANSAC, Image warping and blending
********************************************************************************************************************************************/
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/stitching.hpp>

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

int main(int argc, char ** argv) 
{
  // Read images from directory
  vector < Mat > images;
  string path = "sample_images/*.jpg";
  vector < String > filenames;
  glob(path, filenames);
  for (size_t i = 0; i < filenames.size(); ++i) 
  {
    Mat img = imread(filenames[i]);
    images.push_back(img);
  }

  // Detect keypoints and compute descriptors
  Ptr < Feature2D > detector = SIFT::create();
  vector < vector < KeyPoint >> keypoints;
  vector < Mat > descriptors;
  for (size_t i = 0; i < images.size(); ++i) 
  {
    vector < KeyPoint > kp;
    Mat desc;
    detector -> detectAndCompute(images[i], noArray(), kp, desc);
    keypoints.push_back(kp);
    descriptors.push_back(desc);
  }

  // Match features between adjacent images
  vector < MatchesInfo > pairwise_matches;
  BFMatcher matcher(NORM_L2);
  for (size_t i = 0; i < images.size() - 1; ++i) 
  {
    vector < DMatch > matches;
    matcher.match(descriptors[i], descriptors[i + 1], matches);
    pairwise_matches.push_back(MatchesInfo());
    pairwise_matches.back().src_img_idx = i;
    pairwise_matches.back().dst_img_idx = i + 1;
    pairwise_matches.back().matches = matches;
  }

  // Estimate homography between image pairs
  vector < CameraParams > cameras;
  Ptr < FeaturesMatcher > features_matcher = makePtr < BestOf2NearestMatcher > (false, 0.3 f);
  HomographyBasedEstimator estimator;
  for (size_t i = 0; i < pairwise_matches.size(); ++i) 
  {
    vector < Point2f > points1, points2;
    for (size_t j = 0; j < pairwise_matches[i].matches.size(); ++j) 
    {
      points1.push_back(keypoints[pairwise_matches[i].src_img_idx][pairwise_matches[i].matches[j].queryIdx].pt);
      points2.push_back(keypoints[pairwise_matches[i].dst_img_idx][pairwise_matches[i].matches[j].trainIdx].pt);
    }
    cameras.push_back(CameraParams());
    estimator.estimateTransform(points1, points2, cameras.back().R, cameras.back().t);
  }

  // Perform RANSAC and image warping
  vector < Mat > masks;
  Ptr < WarperCreator > warper_creator = makePtr < cv::CylindricalWarperCreator > ();
  Ptr < RotationWarper > warper = warper_creator -> create(cameras.size());
  for (size_t i = 0; i < cameras.size(); ++i) 
  {
    Mat_ < float > K;
    cameras[i].K().convertTo(K, CV_32F);
    float swa = warper -> calcSweepAngle(cameras[i].R, cameras[i].t);
    warper -> updateScale(swa, cameras[i].R, cameras[i].t, K);
    masks.push_back(warper -> warp(images[i], K, cameras[i].R, INTER_LINEAR, BORDER_REFLECT));
  }

  // Blend warped images
  Mat pano;
  vector < Point > corners, centers;
  warper -> prepare(corners, centers, masks, pano);
  for (size_t i = 0; i < masks.size(); ++i)
    warper -> process(masks[i], corners[i], pano, centers[i]);

  // Display panorama
  imshow("Panorama", pano);
  waitKey(0);

  return 0;
}
