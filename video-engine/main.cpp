#include <chrono>
#include <iostream>
#include <list>
#include <string>

#include "Processing.NDI.Lib.h"
#include "renderer-passthrough-ndi.h"
#include "source.h"

int main() {

  // All parameters are hardcoded for now
  std::string ndiOutputName = "Video Engine";
  int rendererFRateNum = 15;
  int rendererFRateDen = 1;
  int frameDelays = 2;

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
  const NDIlib_source_t *p_sources =
      NDIlib_find_get_current_sources(pNDI_find, &no_sources);

  std::cout << "Number of sources: " << no_sources << std::endl;

  // Clear the name list
  std::list<std::string> ndiSourceNames;
  ndiSourceNames.clear();

  for (uint32_t i = 0; i < no_sources; i++)
    ndiSourceNames.push_back(p_sources[i].p_ndi_name);

  // Lists all the sources
  int sourceIndex = 0;
  for (std::list<std::string>::iterator it = ndiSourceNames.begin();
       it != ndiSourceNames.end(); it++) {
    std::cout << sourceIndex++ << " : " << *it << std::endl;
  }

  std::list<Source *> sources;
  while (true) {
    std::string selectedSourceIndex;
    std::cout << "Select a source index or (q) to quit: ";
    std::cin >> selectedSourceIndex;
    if (selectedSourceIndex == "q") {
      break;
    }
    sourceIndex = std::stoi(selectedSourceIndex);

    Source *videoSource = new Source();
    videoSource->Init((NDIlib_source_t *)&p_sources[sourceIndex]);
    sources.push_back(videoSource);
  }

  // Start the sources
  for (std::list<Source *>::iterator it = sources.begin(); it != sources.end();
       it++) {
    (*it)->Start();
  }

  RendererBase *renderer = new RendererPassthroughNDI(
      rendererFRateNum, rendererFRateDen, ndiOutputName);

  // Add the sources to the renderer
  for (std::list<Source *>::iterator it = sources.begin(); it != sources.end();
       it++) {
    renderer->AddSource(*it);
  }

  // Start the renderer
  renderer->Start();

  // Ask for user input to stop the program, stop if the user enters 'q'
  char c;
  while (1) {
    std::cin >> c;
    if (c == 'q') {
      break;
    }
  }

  // Stop the renderer
  renderer->Stop();

  // Destroy the NDI finder. We needed to have access to the pointers to
  // p_sources[0]
  NDIlib_find_destroy(pNDI_find);

  // Not required, but nice
  NDIlib_destroy();

  // Delete the sources
  for (std::list<Source *>::iterator it = sources.begin(); it != sources.end();
       it++) {
    delete *it;
  }

  // Delete the renderer
  delete renderer;

  return 0;
}