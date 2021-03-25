#pragma once

#include <atomic>
#include <boost/intrusive/list.hpp>

namespace mq
{
  template <typename T>
  class LFQueue : public boost::intrusive::list_base_hook<>
  {
  public:
    template <typename U>
    struct Node
    {
      Node() = default;
      Node(const U& d) 
      : data(d),
        next(nullptr)
      {}
      ~Node() = default;

      U     data;
      Node* next;
      Node* prev;
    };
    
    LFQueue()
    {
      auto node = new Node<T>();
      m_tail = node;
      m_head = node;
    }
    ~LFQueue() = default;

    void push(const T& data)
    {
      auto newNode = new Node<T>(data);
      newNode->next = m_tail.load(std::memory_order_relaxed);
      m_tail.load(std::memory_order_relaxed)->prev = newNode;

      while(
        !m_tail.compare_exchange_weak(
          newNode->next,
          newNode,
          std::memory_order_release,
          std::memory_order_relaxed
        )
      );
    }
    
    bool pop(T& result)
    {
      auto head = m_head.load();
      if (!head)
        return false;
      while (!m_head.compare_exchange_weak(head, head->prev));
      result = head->data;
      delete head;
      return true;
    }
    
    bool is_linked() { return m_is_linked; }
    void link() { m_is_linked = true; }
    void unlink() { m_is_linked = false; }
    
  private:
    std::atomic<Node<T>*> m_head{nullptr};
    std::atomic<Node<T>*> m_tail{nullptr};
    bool                  m_is_linked{false};
  };
  
  template<std::size_t SIZE=5>
  class QueueStorage
  {
  public:
    QueueStorage()
    {
      for(std::size_t i = 0; i < SIZE; ++i)
      {
        auto q = new LFQueue<std::string>();
        m_queues.push_back(*q);
      }
    }
    ~QueueStorage() = default;
    
    QueueStorage(const QueueStorage&) = delete;
    QueueStorage& operator=(const QueueStorage&) = delete;
    
    LFQueue<std::string>* acquire_queue()
    {
      for(auto& q : m_queues)
        if(!q.is_linked())
          return &q;
      auto q = new LFQueue<std::string>();
      m_queues.push_back(*q);
      return q;
    }
    
  private:
    boost::intrusive::list<
      LFQueue<std::string>
    > m_queues;
  };
  
} // namespace mq
