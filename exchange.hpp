#pragma once 

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "queue.hpp"

namespace mq
{
  class Exchange
  {
  public:
    Exchange() = default;
    Exchange(std::string& name, Queue* q)
    : m_name(std::move(name)),
      m_pQueue(std::move(q))
    { m_pQueue->link(); std::cout << __FUNCTION__ << "\n"; }
    
    void publish(std::string& data) { std::cout << __FUNCTION__ << "\n"; m_pQueue->push(data); }
    
    std::string const& name() const { return m_name; }
    
    ~Exchange()
    {
      std::cout << __FUNCTION__ << "\n";
      m_pQueue->unlink();
      delete m_pQueue;
    }
    
  private:
    std::string m_name;
    Queue* m_pQueue;
  };
  
  /*struct exch_name_key
  {
    using type = std::string;

    const type& operator()(const std::shared_ptr<Exchange>& v) const
    { return v->name(); }
  };*/
  
  using exchanges_map_t = std::map<std::string, std::shared_ptr<Exchange>>;
  
}//namespace mq
