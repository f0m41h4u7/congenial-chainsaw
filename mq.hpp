#pragma once

#include <map>

#include "exchange.hpp"
#include "request.hpp"
#include "server.hpp"

#define DEFAULT_PORT 31337

namespace mq
{
  constexpr std::string_view ok_response = "OK\n";
  constexpr std::string_view parse_error = "Error: failed to parse request\n";
  constexpr std::string_view queue_error = "Error: queue is already used\n";
  
  class Router
  {
  public:
    Router() = default;
    ~Router() = default;
    
    std::shared_ptr<Exchange> queue_connect(std::string& name)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      if(m_exchanges.contains(name))
      {
        auto exch_ptr = m_exchanges.at(name);
        std::cout << exch_ptr.use_count() << " EXISTS\n";
        mlock.unlock();
        return exch_ptr;
      }
      m_exchanges[name] = std::make_shared<Exchange>(name, std::move(m_queueStorage.acquire_queue()));
      return m_exchanges[name];
    }
    
    void run()
    {
      boost::asio::io_service svc;
      
      Server s(
        DEFAULT_PORT,
        svc,
        [&](std::string_view data, std::shared_ptr<Conn> conn) -> std::string
        {
          Request req;
          auto err = req.parseAndValidate(data);
          if(err == ErrorCode::ERROR)
            return parse_error.data();
          
          switch(req.m_method)
          {
            case Method::QUEUE_CONNECT:
            {
              conn->set_exchange(queue_connect(req.m_queue));
              return ok_response.data();
            }
            
            default:
              return ok_response.data();
          }
        },
        [&](const std::string& name)
        {
          std::unique_lock<std::mutex> mlock(m_mutex);
          m_exchanges.erase(name);
        }
      );
      
      svc.run();
    }
  
  private:
    std::mutex                                       m_mutex;
    QueueStorage<5>                                  m_queueStorage;
    std::map<std::string, std::shared_ptr<Exchange>> m_exchanges;
  };

} //namespace mq
