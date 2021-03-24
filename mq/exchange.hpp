#pragma once 

#include <iostream>

#include "queue.hpp"

namespace mq
{
  class Exchange
  {
  public:
    Exchange() = default;
    Exchange(std::string& name, LFQueue<std::string>* q)
    : m_name(name),
      m_pQueue(std::move(q))
    { std::cout << __FUNCTION__ << "\n"; m_pQueue->link(); }
    
    void publish(std::string& data)
    {
      m_pQueue->push(data); 
    }
    
    bool receive(std::string& data)
    {
      return m_pQueue->pop(data);
    }
    
    std::string const& name() const { return m_name; }
    
    ~Exchange()
    {
      std::cout << __FUNCTION__ << "\n";
      m_pQueue->unlink();
    }
    
  private:
    std::string m_name;
    LFQueue<std::string>*      m_pQueue;
  };
  
}//namespace mq
