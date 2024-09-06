#ifndef RENDERED_PASSTHROUGH_NDI_HPP___
#define RENDERED_PASSTHROUGH_NDI_HPP___

#include "Processing.NDI.Lib.h"
#include "renderer-base.h"

class RendererPassthroughNDI : public RendererBase {
 public:
  RendererPassthroughNDI(int rendererFRateNum, int rendererFRateDen,
                         std::string ndiSourceName)
      : RendererBase(rendererFRateNum, rendererFRateDen),
        mNDISourceName(ndiSourceName) {
    NDIlib_send_create_t NDI_send_create_desc;
    NDI_send_create_desc.p_ndi_name = nullptr;
    NDI_send_create_desc.p_groups = nullptr;
    // NDI_send_create_desc.clock_video = true;
    // NDI_send_create_desc.clock_audio = false;

    // Output NDI_send_create_desc
    // std::cout << "NDI Source Name: " << NDI_send_create_desc.p_ndi_name
    //           << std::endl;

    mNDISender = NDIlib_send_create(&NDI_send_create_desc);
  }
  virtual ~RendererPassthroughNDI() {
    if (mNDISender) {
      NDIlib_send_destroy(mNDISender);
    }
  }

  void Process(std::vector<NDIlib_video_frame_v2_t> frames) override {
    for (auto frame : frames) {
      NDIlib_send_send_video_v2(mNDISender, &frame);
    }
  }

 private:
  NDIlib_send_instance_t mNDISender;
  std::string mNDISourceName;
};

#endif  // RENDERED_PASSTHROUGH_NDI_HPP___