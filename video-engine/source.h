#ifndef SOURCE_HPP___
#define SOURCE_HPP___

// #include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "Processing.NDI.Lib.h"
#include "tcb.h"

class Source {
 public:
  Source() : mSource(nullptr), mBuffer(8) {}
  virtual ~Source() {}
  void Init(NDIlib_source_t* source) {
    mSource = source;
    // mThread = std::thread(&Source::Run, this);
  }

  bool isRunning() { return mIsRunning; }

  void Start(int runForIterations = 10) {
    mIsRunning = true;
    mThread = std::thread(&Source::Run, this);
  }
  void Stop() {
    mIsRunning = false;
    mThread.join();
  }

  void Wait() { mThread.join(); }

 private:
  std::string mSourceName;
  std::thread mThread;
  bool mIsRunning;

  TimedCircularBuffer<NDIlib_video_frame_v2_t> mBuffer;

  // The source
  NDIlib_source_t* mSource;

  // Write OuputVideoFrame and OutputAudioFrame functions using std::cout

  void OutputVideoFrame(NDIlib_video_frame_v2_t* frame) {
    // Output the frame
    std::cout << "Video data received (" << frame->xres << "x" << frame->yres
              << ") frame rate "
              << (double)frame->frame_rate_N / (double)frame->frame_rate_D
              << " fps | tc " << frame->timecode << " | ts " << frame->timestamp
              << " | ts - tc " << frame->timestamp - frame->timecode << "."
              << std::endl;
    std::cout << "   Unix timecode: " << frame->timecode / 10000000
              << std::endl;
    std::cout << "   Unix timecode (remainder): " << frame->timecode % 10000000
              << std::endl;
  }
  void OutputAudioFrame(NDIlib_audio_frame_v2_t* frame) {
    std::cout << "Audio data received (" << frame->no_samples << " samples)."
              << std::endl;
  }

  void Run() {
    // Test if initialized
    if (!mSource) {
      std::cerr << "Source is not initialized" << std::endl;
      return;
    }

    // We now have at least one source, so we create a receiver to look at
    // it.
    NDIlib_recv_instance_t pNDI_recv = NDIlib_recv_create_v3();
    if (!pNDI_recv) return;

    // Connect to our sources
    NDIlib_recv_connect(pNDI_recv, mSource);

    // Run for five minutes
    using namespace std::chrono;
    for (const auto start = high_resolution_clock::now();
         high_resolution_clock::now() - start <
         minutes(5);) {  // The descriptors

      // Break if not running
      if (!isRunning()) {
        break;
      }

      NDIlib_video_frame_v2_t video_frame;
      NDIlib_audio_frame_v2_t audio_frame;

      Element<NDIlib_video_frame_v2_t> r;

      // Set the deleter
      mBuffer.SetDeleter([pNDI_recv](NDIlib_video_frame_v2_t* frame) {
        NDIlib_recv_free_video_v2(pNDI_recv, frame);
      });

      switch (NDIlib_recv_capture_v2(pNDI_recv, &video_frame, &audio_frame,
                                     nullptr, 5000)) {  // No data
        case NDIlib_frame_type_none:
          printf("No data received.\n");
          break;

        // Video data
        case NDIlib_frame_type_video:
          OutputVideoFrame(&video_frame);
          r = mBuffer.Put(video_frame, video_frame.timestamp);
          mBuffer.Output();
          break;

        // Audio data
        case NDIlib_frame_type_audio:
          OutputAudioFrame(&audio_frame);
          NDIlib_recv_free_audio_v2(pNDI_recv, &audio_frame);
          break;
      }
    }

    // Destroy the receiver
    NDIlib_recv_destroy(pNDI_recv);
  }
};

#endif  // SOURCE_HPP___
