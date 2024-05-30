#include "convert.h"

#include <iostream>

cv::Mat* Convert_NDIlib_video_frame_v2_t_to_CVMat(
    NDIlib_video_frame_v2_t* p_ndi_frame) {
  // Check if the frame is valid
  if (!p_ndi_frame) {
    std::cout << "Invalid frame" << std::endl;
    return;
  }

  // Check if the frame is BGRX
  if (p_ndi_frame->FourCC != NDIlib_FourCC_type_BGRA) {
    std::cout << "Frame format not supported" << std::endl;
    return;
  }

  // Dynamically create a CV::Mat object with the same dimensions as the NDI
  // frame
  cv::Mat* cvMat = new cv::Mat(p_ndi_frame->yres, p_ndi_frame->xres, CV_8UC4,
                               p_ndi_frame->p_data);

  return cvMat;
}