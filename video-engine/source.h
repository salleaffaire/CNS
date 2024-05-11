#ifndef SOURCE_HPP___
#define SOURCE_HPP___

// #include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>

#include "Processing.NDI.Lib.h"

template <typename T>
class Element {
 public:
  T mItem;
  uint64_t mTimestamp;
};

template <typename T>
class TimedCircularBuffer {
 public:
  TimedCircularBuffer()
      : mBuffer(nullptr), mSize(0), mCurrentWrite(0), mCurrentRead(0) {}
  TimedCircularBuffer<T>(int size) { Init(size); }
  virtual ~TimedCircularBuffer<T>() { Deinit(); }

  void Init(int size) {
    mSize = size;
    mBuffer = new Element<T>[mSize];
    mCurrentWrite = 0;
    mCurrentRead = 0;
    // Intialize all timestamps in the buffer to -1
    for (int i = 0; i < mSize; i++) {
      mBuffer[i].mTimestamp = -1;
    }
  }

  void Deinit() {
    if (mBuffer) {
      delete[] mBuffer;
    }
  }

  void Put(T item, uint64_t timestamp) {
    // Check if is init or not
    if (!mBuffer) {
      std::cerr << "CircularBuffer is not initialized" << std::endl;
      return;
    }
    std::unique_lock<std::mutex> lock(mMutex);
    mBuffer[mCurrentWrite % mSize] = Element<T>{item, timestamp};
    mCurrentWrite++;
    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    // lock.unlock();
    // mCond.notify_one();
  }

  T Get(uint64_t timestamp, int threshold) {
    // Check if is init or not
    if (!mBuffer) {
      std::cerr << "CircularBuffer is not initialized" << std::endl;
      return;
    }
    std::unique_lock<std::mutex> lock(mMutex);
    // Wait until there is data in the buffer, when mCurrentWrite > mCurrentRead
    // mCond.wait(lock, [this] { return mCurrentWrite > mLastReadIndex; });

    // Find the index of the item withiin the threshold of the timestamp
    // starting from the current read index
    int index = mCurrentRead;
    while (index < mCurrentWrite) {
      if (std::abs(mBuffer[index % mSize].mTimestamp - timestamp) >=
          threshold) {
        break;
      }
      index++;
    }
    // This will be wrong if we don't find any item within the threshold
    return mBuffer[index % mSize].mItem;
  }

 private:
  Element<T>* mBuffer;
  int mSize;
  int mCurrentWrite;
  int mCurrentRead;

  std::mutex mMutex;
  // std::condition_variable mCond;
};

class Source {
 public:
  Source();
  virtual ~Source();
  void Init(std::string sourceName);
  void Start();
  void Stop();

 private:
  std::string mSourceName;
  std::thread mThread;

  TimedCircularBuffer<int> mBuffer;

  void Run(const NDIlib_source_t* sources) {
    // We now have at least one source, so we create a receiver to look at it.
    NDIlib_recv_instance_t pNDI_recv = NDIlib_recv_create_v3();
    if (!pNDI_recv) return;

    // Connect to our sources
    NDIlib_recv_connect(pNDI_recv, sources);

    // Run for one minute
    using namespace std::chrono;
    for (const auto start = high_resolution_clock::now();
         high_resolution_clock::now() - start <
         minutes(5);) {  // The descriptors
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
              (double)video_frame.frame_rate_N /
                  (double)video_frame.frame_rate_D,
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
  }
};

#endif  // SOURCE_HPP___
