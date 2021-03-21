#pragma once 

#include <iostream>

#include "queue.hpp"

namespace mq
{
  class Exchange
  {
  public:
    Exchange() = default;
    Exchange(std::string& name, Queue* q)
    : m_name(name),
      m_pQueue(std::move(q))
    { std::cout << __FUNCTION__ << "\n"; m_pQueue->link(); }
    
    void publish(std::string& data) { std::cout << __FUNCTION__ << "\n"; m_pQueue->push(data); }
    std::string receive() {  std::cout << __FUNCTION__ << "\n"; return m_pQueue->pop(); }
    
    std::string const& name() const { return m_name; }
    
    ~Exchange()
    {
      std::cout << __FUNCTION__ << "\n";
      m_pQueue->unlink();
    }
    
  private:
    std::string m_name;
    Queue*      m_pQueue;
  };
  
}//namespace mq
