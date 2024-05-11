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

  // We now have at least one source, so we create a receiver to look at it.
  NDIlib_recv_instance_t pNDI_recv = NDIlib_recv_create_v3();
  if (!pNDI_recv) return 0;

  // Connect to our sources
  NDIlib_recv_connect(pNDI_recv, p_sources + 0);

  // Destroy the NDI finder. We needed to have access to the pointers to
  // p_sources[0]
  NDIlib_find_destroy(pNDI_find);

  // Run for one minute
  using namespace std::chrono;
  for (const auto start = high_resolution_clock::now();
       high_resolution_clock::now() - start < minutes(5);) {  // The descriptors
    NDIlib_video_frame_v2_t video_frame;
    NDIlib_audio_frame_v2_t audio_frame;

    switch (NDIlib_recv_capture_v2(pNDI_recv, &video_frame, &audio_frame,
                                   nullptr, 5000)) {  // No data
      case NDIlib_frame_type_none:
        printf("No data received.\n");
        break;

      // Video data
      case NDIlib_frame_type_video:
        printf(
            "Video data received (%dx%d) frame rate %f fps | tc %ld | ts "
            "%ld | ts - tc %ld.\n",
            video_frame.xres, video_frame.yres,
            (double)video_frame.frame_rate_N / (double)video_frame.frame_rate_D,
            video_frame.timecode, video_frame.timestamp,
            video_frame.timestamp - video_frame.timecode);
        printf("   Unix timecode: %ld\n", video_frame.timecode / 10000000);
        printf("   Unix timecode (remainder): %ld\n",
               video_frame.timecode % 10000000);
        NDIlib_recv_free_video_v2(pNDI_recv, &video_frame);
        break;

      // Audio data
      case NDIlib_frame_type_audio:
        printf("Audio data received (%d samples).\n", audio_frame.no_samples);
        NDIlib_recv_free_audio_v2(pNDI_recv, &audio_frame);
        break;
    }
  }

  // Destroy the receiver
  NDIlib_recv_destroy(pNDI_recv);

  // Not required, but nice
  NDIlib_destroy();

  return 0;
}