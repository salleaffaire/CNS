#ifndef CONVERT_HPP___
#define CONVERT_HPP___

#include <opencv2/opencv.hpp>

#include "Processing.NDI.Lib.h"

cv::Mat* Convert_NDIlib_video_frame_v2_t_to_CVMat(
    NDIlib_video_frame_v2_t* p_ndi_frame);

#endif