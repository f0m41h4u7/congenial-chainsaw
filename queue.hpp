#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace mq
{
  template <typename T, std::size_t BUFFER_SIZE=1024>
  class cqueue
  {
  public:
    cqueue() = default;
    ~cqueue() = default;

    cqueue(const cqueue&) = delete;
    cqueue& operator=(const cqueue&) = delete;

    void pop(T& item)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      while (m_queue.empty())
        m_cv.wait(mlock);
    
      item = m_queue.front();
      m_queue.pop();
      mlock.unlock();
      m_cv.notify_one();                      
    }

    void push(const T& item)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      while (m_queue.size() >= BUFFER_SIZE)
        m_cv.wait(mlock);      
      m_queue.push(item);
      mlock.unlock();
      m_cv.notify_one();                  
    }

    T pop()
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      while (m_queue.empty())
        m_cv.wait(mlock);

      auto val = m_queue.front();
      m_queue.pop();
      mlock.unlock();
      m_cv.notify_one();

      return val;
    }

  private:
    std::queue<T> m_queue;

    std::mutex m_mutex;
    std::condition_variable m_cv;
  };
} // namespace
