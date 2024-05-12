#ifndef TIMED_CIRCULAR_BUFFER_HPP___
#define TIMED_CIRCULAR_BUFFER_HPP___

#include <functional>
#include <iostream>
#include <mutex>

template <typename T>
class Element {
 public:
  Element() : mItem(), mTimestamp(-1), mIsSet(false) {}
  Element(T item, uint64_t timestamp, bool isSet)
      : mItem(item), mTimestamp(timestamp), mIsSet(isSet) {}
  T mItem;
  uint64_t mTimestamp;
  bool mIsSet;
};

// Overload the << operator for the Element class
std::ostream& operator<<(std::ostream& os, const Element<int>& element) {
  os << element.mItem << " " << element.mTimestamp << " " << element.mIsSet
     << std::endl;
  return os;
}

template <typename T>
class TimedCircularBuffer {
 public:
  TimedCircularBuffer()
      : mBuffer(nullptr),
        mSize(0),
        mCurrentWrite(0),
        mCurrentRead(0),
        mDeleter(nullptr) {}
  TimedCircularBuffer<T>(int size)
      : mBuffer(nullptr),
        mSize(0),
        mCurrentWrite(0),
        mCurrentRead(0),
        mDeleter(nullptr) {
    Init(size);
  }
  virtual ~TimedCircularBuffer<T>() { Deinit(); }

  void Init(int size) {
    Deinit();
    mBuffer = new Element<T>[size];
    mSize = size;
    mCurrentWrite = 0;
    mCurrentRead = 0;
  }

  void Deinit() {
    // Apply the deleter to all the elements
    if (mBuffer && mDeleter) {
      for (int i = 0; i < mSize; i++) {
        if (mBuffer[i].mIsSet) {
          mDeleter(&mBuffer[i].mItem);
        }
      }
    }
    if (mBuffer) {
      delete[] mBuffer;
    }
  }

  void SetDeleter(std::function<void(T*)> deleter) { mDeleter = deleter; }

  Element<T> Put(T item, uint64_t timestamp) {
    // Check if is init or not
    if (!mBuffer) {
      std::cerr << "CircularBuffer is not initialized" << std::endl;
      return Element<T>();
    }
    std::unique_lock<std::mutex> lock(mMutex);

    Element<T> rval = mBuffer[mCurrentWrite % mSize];

    // Apply the deleter to the item if it is set
    if (rval.mIsSet && mDeleter) {
      mDeleter(&rval.mItem);
    }

    mBuffer[mCurrentWrite % mSize] = Element<T>{item, timestamp, true};
    mCurrentWrite++;
    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    // lock.unlock();
    // mCond.notify_one();
    return rval;
  }

  T Get(uint64_t timestamp, int threshold) {
    // Check if is init or not
    if (!mBuffer) {
      std::cerr << "CircularBuffer is not initialized" << std::endl;
      return T();
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

  void Output() {
    std::unique_lock<std::mutex> lock(mMutex);
    for (int i = 0; i < mSize; i++) {
      if (mBuffer[i].mIsSet) {
        if (i == mCurrentWrite % mSize) {
          std::cout << "X";
        } else {
          std::cout << "x";
        }
      } else {
        std::cout << "O";
      }
    }
    std::cout << std::endl;
  }

 private:
  Element<T>* mBuffer;
  int mSize;
  int mCurrentWrite;
  int mCurrentRead;

  std::mutex mMutex;

  // The deleter, is a function pointer that takes a T* as a parameter
  std::function<void(T*)> mDeleter;

  // std::condition_variable mCond;
};

#endif  // TIMED_CIRCULAR_BUFFER_HPP___