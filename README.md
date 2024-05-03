# Panorama-Stiching-OpenCV
This C++ code uses the OpenCV library for image processing and stitching. 
- The `load_images` function loads all image files (with .jpg or .png extension) from the specified folder path. 
- The `resize_images` function resizes the loaded images to the specified width and height. 
- The `stitch_images` function uses the OpenCV Stitcher class to stitch the resized images together.
- The `crop_image` function crops the stitched image by finding the largest contour and extracting the bounding rectangle.
- The `preview_and_save_image` function displays the cropped image and saves it to the specified folder with the folder name appended to the filename. 
- The `stitch_folder` function orchestrates the entire process by calling the other functions in the correct order.
