#pragma once

#include <boost/intrusive/list.hpp>
#include <condition_variable>
#include <mutex>
#include <queue>

#define BUFFER_SIZE 1024

namespace mq
{
  class Queue : public boost::intrusive::list_base_hook<>
  {
  public:
    Queue() = default;
    ~Queue() = default;

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void pop(std::string& item)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      while (m_queue.empty())
        m_cv.wait(mlock);
    
      item = m_queue.front();
      m_queue.pop();
      mlock.unlock();
      m_cv.notify_one();                      
    }

    void push(const std::string& item)
    {
      std::cout << __FUNCTION__ << "QUEUE: " << item << "\n";
      std::unique_lock<std::mutex> mlock(m_mutex);
      while (m_queue.size() >= BUFFER_SIZE)
        m_cv.wait(mlock);      
      m_queue.push(item);
      mlock.unlock();
      m_cv.notify_one();                  
    }

    std::string pop()
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
    
    bool is_linked() { return m_is_linked; }
    void link() { std::cout << __FUNCTION__ << "\n"; m_is_linked = true; }
    void unlink() { std::cout << __FUNCTION__ << "\n"; m_is_linked = false; }

  private:
    bool                    m_is_linked{false};
    std::queue<std::string> m_queue;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
  };
  
  template<std::size_t SIZE=10>
  class QueueStorage
  {
  public:
    QueueStorage()
    {
      for(std::size_t i = 0; i < SIZE; ++i)
      {
        auto q = new Queue();
        m_queues.push_back(*q);
      }
    }
    ~QueueStorage() = default;
    
    Queue* acquire_queue()
    {
      std::cout << __FUNCTION__ << "\n";
      for(auto& q : m_queues)
        if(!q.is_linked())
          return &q;
      auto q = new Queue();
      m_queues.push_back(*q);
      return q;
    }
    
  private:
    boost::intrusive::list<
      Queue
    > m_queues;
  };
  
} // namespace mq
