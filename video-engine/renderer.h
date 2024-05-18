#ifndef RENDERER_HPP___
#define RENDERER_HPP___

#include <cstdlib>
#include <thread>
#include <vector>

#include "source.h"

class Renderer {
 public:
  Renderer() : mIsRunning(false) {}
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
  bool mIsRunning;

  std::vector<Source*> mSources;

  void Run() {
    // If there is one source then we can start the rendering loop
    if (mSources.size() == 0) {
      std::cout << "No sources to render" << std::endl;
      return;
    }
    std::cout << "Sources to render: " << mSources.size() << std::endl;

    // Simulate a renderering loop
    // Frame per second
    const double fpsOut = 120;
    const double frameDurationOut = 1.0 * 1000 / fpsOut;    // in milliseconds
    const int frameDurationOutInt = (int)frameDurationOut;  // in milliseconds

    const double fpsIn = 60;
    const double frameDurationIn = 1.0 * 1000 / fpsIn;    // in milliseconds
    const int frameDurationInInt = (int)frameDurationIn;  // in milliseconds

    while (mIsRunning) {
      // Output current system timestamp (now)
      uint64_t now =
          std::chrono::system_clock::now().time_since_epoch().count();

      // We want to be 2 frame behind the current frame
      now -= (frameDurationInInt * 1000000);
      // The NDI timestamp is in 100ns intervals
      // The now timestamp is in ns intervals
      std::cout << "Current system timestamp: " << now / 100 << std::endl;

      // The 2 parameters are
      // 1. The timestamp in 100ns intervals
      // 2. The threshold in 100ns intervals
      mSources[0]->GetVideoFrameAtTime(now / 100, frameDurationInInt * 10000);

      std::this_thread::sleep_for(
          std::chrono::milliseconds(frameDurationOutInt));
    }

    // We need to stop all the sources only one for now
    mSources[0]->Stop();
  }
};

#endif  // RENDERER_HPP___