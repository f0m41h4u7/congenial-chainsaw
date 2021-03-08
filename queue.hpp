#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace mq
{
 /* template <typename T>
  class node
  {
  public:
    explicit node(const T& val) : m_value(val) {}

    template <typename... Args>
    node(Args&&... args)
    : m_next(nullptr),
      m_value(std::forward<Args>(args)...)
    {}*/
/*
    ~node() = default;

    T& get() { return m_value;  }
    Node<T>* next() { return m_next;  }
    void set_next(Node<T>* ptr) { m_next = ptr;  }

  private:
    node<T>* m_pHead{nullptr};
    T        m_value;
  };

  template <typename T>
  class lfqueue
  {
  public:
    lfqueue() = default;
    ~lfqueue() = default;

    using node_ptr = node<T>*;

    void push(const T& data)
    {
      node_ptr new_node = new node<T>(data);
      new_node->set_next() = m_tail.load(std::memory_order_relaxed);

      while(
          !m_head.compare_exchange_weak(
            new_node->next(),
            new_node,
            std::memory_order_release,
            std::memory_order_relaxed
            )
          );
      std::cout << "Head now is: " << m_head.load()->data << std::endl;
      if (m_head.load()->next)
        std::cout << "And next is: " << m_head.load()->next->data << std::endl;
    }

    bool pop(T& result)
    {
      node_ptr prev_head = m_head.load();
      if (!prev_head)
        return false;
      
      while (!m_head.compare_exchange_weak(prev_head, prev_head->next()));
      result = prev_head->data;
      delete prev_head;
      return true;                  
    }

  private:
    std::size_t m_size{0};
    std::atomic<node_ptr> m_tail{nullptr};
    std::atomic<node<T>> m_head;
  };

*/
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
