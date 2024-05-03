#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cuda_runtime.h>

using namespace std;
using namespace cv;

// CUDA kernel for image stitching
__global__ void stitchImages(const uchar4 *
  const * inputImages, uchar4 * outputImage, int width, int height, int numImages) 
  {
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;

  if (x < width && y < height) 
  {
    int imageIndex = x / (width / numImages);
    int imageX = x % (width / numImages);
    int imageY = y;

    outputImage[y * width + x] = make_uchar4(inputImages[imageIndex][imageY * (width / numImages) + imageX].x,
      inputImages[imageIndex][imageY * (width / numImages) + imageX].y,
      inputImages[imageIndex][imageY * (width / numImages) + imageX].z,
      inputImages[imageIndex][imageY * (width / numImages) + imageX].w);
  }
}

int main() 
{
  // Read images from a folder
  vector < Mat > images;
  string folderPath = "sample_images";
  vector < String > filenames;
  glob(folderPath, filenames);

  for (const auto & filename: filenames) 
  {
    Mat image = imread(filename, IMREAD_COLOR);
    if (!image.empty()) 
    {
      images.push_back(image);
    }
  }

  int numImages = images.size();
  int width = 0, height = 0;

  // Calculate the total width and height of the panorama
  for (const auto & image: images) 
  {
    width += image.cols;
    height = max(height, image.rows);
  }

  // Allocate memory for input and output images
  vector < uchar4 * > inputImageData(numImages);
  uchar4 * outputImageData;
  cudaMallocManaged( & outputImageData, width * height * sizeof(uchar4));

  // Copy input images to device memory
  for (int i = 0; i < numImages; i++) 
  {
    cudaMallocManaged( & inputImageData[i], images[i].total() * images[i].elemSize());
    cudaMemcpy(inputImageData[i], images[i].data, images[i].total() * images[i].elemSize(), cudaMemcpyHostToDevice);
  }

  // Launch CUDA kernel
  dim3 blockSize(16, 16);
  dim3 gridSize((width + blockSize.x - 1) / blockSize.x, (height + blockSize.y - 1) / blockSize.y);
  stitchImages << < gridSize, blockSize >>> (inputImageData.data(), outputImageData, width, height, numImages);
  cudaDeviceSynchronize();

  // Copy output image to host memory
  Mat outputImage(height, width, CV_8UC4, outputImageData);

  // Display the panorama image
  imshow("CUDA Panorama", outputImage);
  waitKey(0);

  // Free device memory
  cudaFree(outputImageData);
  for (int i = 0; i < numImages; i++) 
  {
    cudaFree(inputImageData[i]);
  }

  return 0;
}
