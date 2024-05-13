#ifndef TIMED_CIRCULAR_BUFFER_HPP___
#define TIMED_CIRCULAR_BUFFER_HPP___

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

template <typename T>
class Element {
 public:
  Element() : mItem(), mTimestamp(-1), mIsSet(false), mIsLocked(false) {}
  Element(T item, uint64_t timestamp, bool isSet, bool isLocked = false)
      : mItem(item),
        mTimestamp(timestamp),
        mIsSet(isSet),
        mIsLocked(isLocked) {}
  T mItem;
  uint64_t mTimestamp;
  bool mIsSet;
  bool mIsLocked;
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

  void Put(T item, uint64_t timestamp) {
    // Check if is init or not
    if (!mBuffer) {
      std::cerr << "CircularBuffer is not initialized" << std::endl;
      return;
    }

    // Declare a lock
    std::unique_lock<std::mutex> lock(mMutex);

    // Wait until the element is unlocked
    mCond.wait(lock,
               [this] { return !mBuffer[mCurrentWrite % mSize].mIsLocked; });

    // Write index
    const int index = mCurrentWrite % mSize;

    // Apply the deleter to the item if it is set
    Element<T> val = mBuffer[index];
    if (val.mIsSet && mDeleter) {
      mDeleter(&val.mItem);
    }

    // Set the item
    mBuffer[index] = Element<T>{item, timestamp, true};
    mCurrentWrite++;

    return;
  }

  T Get(uint64_t timestamp, int threshold) {
    // Check if is init or not
    if (!mBuffer) {
      std::cerr << "CircularBuffer is not initialized" << std::endl;
      return T();
    }
    std::unique_lock<std::mutex> lock(mMutex);

    // If ReadIndex is smaller than WriteIndex - 8, reset the read index
    if (mCurrentRead < mCurrentWrite - mSize) {
      mCurrentRead = mCurrentWrite - (mSize / 2);
    }

    // Find the index of the item within the threshold of the timestamp
    // starting from the current read index
    int index = mCurrentRead;
    while (index < mCurrentWrite) {
      std::cout << "mCurrentWrite: " << mCurrentWrite
                << " | mCurrentRead: " << mCurrentRead << " | index: " << index
                << std::endl;
      // std::cout << "Buffer Timestamp: " << mBuffer[index % mSize].mTimestamp
      //           << std::endl;
      // std::cout << "Timestamp       : " << timestamp << std::endl;
      uint64_t diff = std::abs(int64_t(mBuffer[index % mSize].mTimestamp) -
                               int64_t(timestamp));

      std::cout << "Diff: " << diff << " | Threshold: " << threshold
                << std::endl;
      if (diff <= threshold) {
        mCurrentRead = index;
        std::cout << "ReadIndex: " << mCurrentRead << std::endl;
        break;
      }
      index++;
    }
    // This will be wrong if we don't find any item within the threshold
    return mBuffer[index % mSize].mItem;
  }

  void Unlock(int index) {
    std::unique_lock<std::mutex> lock(mMutex);
    mBuffer[index].mIsLocked = false;
    mCond.notify_one();
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

  std::condition_variable mCond;
};

#endif  // TIMED_CIRCULAR_BUFFER_HPP___
