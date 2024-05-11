#ifndef SOURCE_HPP___
#define SOURCE_HPP___

#include <string>
#include <thread>

template <typename T> class CircularBuffer {
public:
  CircularBuffer<T>();
  virtual ~CircularBuffer<T>();

  void Init(int size) {
    mSize = size;
    mBuffer = new T[mSize];
    mCurrentWrite = 0;
    mCurrentRead = 0;
  }

  void Put(T item) {
    std::unique_lock<std::mutex> lock(mMutex);
    mCond.wait(lock, [this] { return mCount < mSize; });
  }

  T Get() {
    std::unique_lock<std::mutex> lock(mMutex);
    mCond.wait(lock, [this] { return mCount > 0; });
  }

private:
  T *mBuffer;
  int mSize;

  std::mutex mMutex;
  std::condition_variable mCond;
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
  int mCurrentWrite;
  int mCurrentRead;

  void Run() const;
};

#endif // SOURCE_HPP___
