#pragma once 

#include <iostream>

#include "queue.hpp"

namespace mq
{
  enum State { DEFAULT, CONSUMING };
  
  class Exchange
  {
  public:
    Exchange() = default;
    Exchange(std::string& name, Queue* q)
    : m_name(name),
      m_pQueue(std::move(q))
    { std::cout << __FUNCTION__ << "\n"; m_pQueue->link(); }
    
    void publish(std::string& data) { m_pQueue->push(data); }
    std::string receive() { return m_pQueue->pop(); }
    
    std::string const& name() const { return m_name; }
    
    State state() const { return m_state; }
    void set_state(State s) { m_state = s; }
    
    ~Exchange()
    {
      std::cout << __FUNCTION__ << "\n";
      m_pQueue->unlink();
    }
    
  private:
    State       m_state{DEFAULT};
    std::string m_name;
    Queue*      m_pQueue;
  };
  
}//namespace mq
