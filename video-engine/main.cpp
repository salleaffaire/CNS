#include <chrono>
#include <iostream>
#include <list>
#include <string>

#include "Processing.NDI.Lib.h"
#include "source.h"

int main() {
  std::cout << "Starting Video Engine ..." << std::endl;

  // Discovering NDI sources
  // Not required, but "correct" (see the SDK documentation).
  if (!NDIlib_initialize()) {
    std::cout << "Cannot run NDIlib_initialize()" << std::endl;
    return -1;
  }
  // We are going to create an NDI finder that locates sources on the network.
  NDIlib_find_instance_t pNDI_find = NDIlib_find_create_v2();
  if (!pNDI_find) {
    return -1;
  }

  // For some reason, this has to be called first
  if (!NDIlib_find_wait_for_sources(pNDI_find, 2000 /* milliseconds */)) {
    std::cout << "No change to the sources found." << std::endl;
  }

  uint32_t no_sources = 0;
  const NDIlib_source_t* p_sources =
      NDIlib_find_get_current_sources(pNDI_find, &no_sources);

  std::cout << "Number of sources: " << no_sources << std::endl;

  // Clear the name list
  std::list<std::string> NDISources;
  NDISources.clear();

  for (uint32_t i = 0; i < no_sources; i++)
    NDISources.push_back(p_sources[i].p_ndi_name);

  // Lists all the sources
  int sourceIndex = 0;
  for (std::list<std::string>::iterator it = NDISources.begin();
       it != NDISources.end(); it++) {
    std::cout << sourceIndex << " : " << *it << std::endl;
  }

  // Select a source index
  int selectedSourceIndex = 0;
  std::cout << "Select a source index: ";
  std::cin >> selectedSourceIndex;

  std::cout << "Selected source: " << selectedSourceIndex << std::endl;

  // Check that the selected source index is valid
  if (selectedSourceIndex > (NDISources.size() - 1)) {
    std::cout << "Invalid source index" << std::endl;
    return -1;
  }

  Source videoSource;
  videoSource.Init((NDIlib_source_t*)&p_sources[selectedSourceIndex]);
  videoSource.Start();

  // Simulate a renderering loop
  // Frame per second
  const double fpsOut = 120;
  const double frameDurationOut = 1.0 * 1000 / fpsOut;    // in milliseconds
  const int frameDurationOutInt = (int)frameDurationOut;  // in milliseconds

  const double fpsIn = 60;
  const double frameDurationIn = 1.0 * 1000 / fpsIn;    // in milliseconds
  const int frameDurationInInt = (int)frameDurationIn;  // in milliseconds

  while (1) {
    if (videoSource.isRunning()) {
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
      videoSource.GetVideoFrameAtTime(now / 100, frameDurationInInt * 10000);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(frameDurationOutInt));
  }

  // We need to stop here to prevent the program from exiting
  videoSource.Wait();

  // Destroy the NDI finder. We needed to have access to the pointers to
  // p_sources[0]
  NDIlib_find_destroy(pNDI_find);

  // Not required, but nice
  NDIlib_destroy();

  return 0;
}