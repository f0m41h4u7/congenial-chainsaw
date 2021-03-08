#pragma once

#include "queue.hpp"
#include "server.hpp"

#define DEFAULT_PORT 31337

namespace mq
{
  template <typename T>
  class router
  {
  public:
    router() = default;
    ~router() = default;
    
    void run()
    {
      boost::asio::io_service svc;
      
      net::server s(
        DEFAULT_PORT,
        svc,
        [&](std::string_view req) -> std::string
        {
          publish(std::forward<std::string>(req.data()));
          std::cout << "received: " << req.data() << "\n";
          return OK_RESPONSE;
        }
      );
      
      svc.run();
    }
    
    void publish(T&& data)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      m_queue.push(std::forward<T>(data));
    }
    
    T receive()
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      return m_queue.pop();
    }
  
  private:
    std::mutex m_mutex;
    cqueue<T> m_queue;
    //std::unique_ptr<nett::server> m_server;
  };

} //namespace mq
