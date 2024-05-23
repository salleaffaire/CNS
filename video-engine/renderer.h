#ifndef RENDERER_HPP___
#define RENDERER_HPP___

#include <cstdlib>
#include <thread>
#include <vector>

#include "source.h"

class Renderer {
 public:
  Renderer(int rendererFRateNum, int rendererFRateDen)
      : mIsRunning(false),
        mRendererFRateNum(rendererFRateNum),
        mRendererFRateDen(rendererFRateDen) {}
  virtual ~Renderer() {}

  void Start() {
    mIsRunning = true;
    mThread = std::thread(&Renderer::Run, this);
  }

  void Stop() {
    mIsRunning = false;
    mThread.join();
  }

  void AddSource(Source* source) { mSources.push_back(source); }

 private:
  std::thread mThread;

  int mRendererFRateDen, mRendererFRateNum;

  bool mIsRunning;

  std::vector<Source*> mSources;

  // Assume only one source for now
  void Run() {
    // If there is one source then we can start the rendering loop
    if (mSources.size() == 0) {
      std::cout << "No sources to render" << std::endl;
      return;
    }
    std::cout << "Sources to render: " << mSources.size() << std::endl;

    // Simulate a renderering loop
    // Frame per second
    const double fpsOut = mRendererFRateNum / mRendererFRateDen;
    const double frameDurationOut = 1.0 * 1000 / fpsOut;    // in milliseconds
    const int frameDurationOutInt = (int)frameDurationOut;  // in milliseconds

    while (mIsRunning) {
      // Output current system timestamp (now)
      uint64_t now =
          std::chrono::system_clock::now().time_since_epoch().count();
      // The NDI timestamp is in 100ns intervals
      // The now timestamp is in ns intervals
      std::cout << "Current system timestamp: " << now / 100 << std::endl;

      // For all the sources
      for (int i = 0; i < mSources.size(); i++) {
        if (mSources[i]->GetSourceFRateDen() == 0) {
          std::cout << "Source frame rate not set" << std::endl;
          continue;
        }
        // Get the input frame rate from the source
        const double fpsIn =
            mSources[i]->GetSourceFRateNum() / mSources[i]->GetSourceFRateDen();
        const double frameDurationIn = 1.0 * 1000 / fpsIn;    // in milliseconds
        const int frameDurationInInt = (int)frameDurationIn;  // in milliseconds

        // We want to be 2 frame behind the current frame in term of input
        // frames
        uint64_t targetTime = now - (2 * (frameDurationInInt * 1000000));

        // The 2 parameters are
        // 1. The timestamp in 100ns intervals
        // 2. The threshold in 100ns intervals
        int index = 0;
        int writeIndex = 0;
        auto frame = mSources[i]->GetVideoFrameAtTime(
            targetTime / 100, frameDurationInInt * 10000, &index, &writeIndex);
        std::cout << "Now : " << now / 100 << " | Target: " << targetTime / 100
                  << " | Source TS: " << frame.timestamp
                  << " | Diff:" << (targetTime / 100 - frame.timestamp)
                  << " | Index: " << index << " | Write Index: " << writeIndex
                  << std::endl;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(frameDurationOutInt));
    }

    // We need to stop all the sources only one for now
    for (int i = 0; i < mSources.size(); i++) {
      mSources[i]->Stop();
    }
  }
};

#endif  // RENDERER_HPP___