#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <iostream>
#include <vector>
#include <string>

std::vector < cv::Mat > load_images(const std::string & folder_path) 
{
  std::vector < cv::Mat > images;
  std::vector < cv::String > filenames;
  cv::glob(folder_path, filenames);

  for (const auto & filename: filenames) 
  {
    // Check if file is an image file
    if (filename.substr(filename.length() - 4) == ".jpg" || filename.substr(filename.length() - 4) == ".png") 
    {
      // Load the image using OpenCV
      cv::Mat image = cv::imread(filename);
      images.push_back(image);
    }
  }
  return images;
}

std::vector < cv::Mat > resize_images(const std::vector < cv::Mat > & images, int width, int height) 
{
  std::vector < cv::Mat > resized_images;
  for (const auto & image: images) 
  {
    cv::Mat resized_image;
    cv::resize(image, resized_image, cv::Size(width, height));
    resized_images.push_back(resized_image);
  }
  return resized_images;
}

cv::Mat stitch_images(const std::vector < cv::Mat > & images) 
{
  cv::Ptr < cv::Stitcher > stitcher = cv::Stitcher::create();
  cv::Mat stitched_image;
  std::vector < cv::Mat > stitched_images;
  stitcher -> stitch(images, stitched_images);
  if (!stitched_images.empty()) {
    stitched_image = stitched_images[0];
  }
  return stitched_image;
}

cv::Mat crop_image(const cv::Mat & image) 
{
  cv::Mat gray, thresh;
  cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
  cv::threshold(gray, thresh, 0, 255, cv::THRESH_BINARY);
  std::vector < std::vector < cv::Point >> contours;
  cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  if (!contours.empty()) 
  {
    cv::Rect boundingRect = cv::boundingRect(contours[0]);
    return image(boundingRect);
  }
  return cv::Mat();
}

void preview_and_save_image(const cv::Mat & image,const std::string & folder_path,const std::string & folder_name) 
{
  // Display the stitched image
  cv::namedWindow("Stitched Image", cv::WINDOW_NORMAL);
  cv::imshow("Stitched Image", image);
  cv::waitKey(0);

  // Save the stitched image
  std::string output_filename = folder_path + "/" + folder_name + "_panorama.jpg";
  cv::imwrite(output_filename, image);
  std::cout << "Stitched image saved for folder: " << folder_name << std::endl;
}

void stitch_folder(const std::string & folder_path, int width = 800, int height = 800) 
{
  // Stitch all images in a folder and save the result.
  // Load the images from the folder
  std::vector < cv::Mat > images = load_images(folder_path);

  // Check if there are at least two images in the folder
  if (images.size() < 2) 
  {
    std::cout << "Not enough images in folder: " << folder_path << std::endl;
    return;
  }

  // Resize the images
  std::vector < cv::Mat > resized_images = resize_images(images, width, height);

  // Stitch the images
  cv::Mat stitched_image = stitch_images(resized_images);
  if (stitched_image.empty()) 
  {
    std::cout << "Stitching failed for folder: " << folder_path << std::endl;
    return;
  }

  // Crop the stitched image
  cv::Mat cropped_image = crop_image(stitched_image);

  // Preview and save the stitched image
  std::string folder_name = folder_path.substr(folder_path.find_last_of("/\\") + 1);
  preview_and_save_image(cropped_image, folder_path, folder_name);
}

int main() 
{
  stitch_folder("sample_images");
  return 0;
}
