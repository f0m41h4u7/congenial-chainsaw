#pragma once

#include <map>

#include "exchange.hpp"
#include "request.hpp"
#include "server.hpp"

namespace mq
{
  const short unsigned int DEFAULT_PORT = 31337;
  
  const std::string ok_response = "OK\n";
  const std::string parse_error = "Error: failed to parse request\n";
  const std::string queue_error = "Error: wrong queue name\n";
  
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
        mlock.unlock();
        return exch_ptr;
      }
      m_exchanges[name] = std::make_shared<Exchange>(name, std::move(m_queueStorage.acquire_queue()));
      return m_exchanges[name];
    }
    
    bool queue_exists(std::string& q_name)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      if(m_exchanges.contains(q_name))
        return true;
      return false;
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
          if(err != ErrorCode::OK)
            return parse_error;
          
          switch(req.m_method)
          {
            case Method::CONNECT:
            {
              conn->set_exchange(queue_connect(req.m_queue));
              return ok_response;
            }
            
            case Method::PUBLISH:
            {
              auto q_name = conn->get_exchange()->name();
              if(queue_exists(q_name))
              {
                conn->get_exchange()->publish(req.m_data);
                return ok_response;
              }
              return queue_error;
            }
            
            case Method::CONSUME:
            {
              auto q_name = conn->get_exchange()->name();
              if(queue_exists(q_name))
              {
                conn->get_exchange()->set_state(State::CONSUMING);
                return ok_response;
              }
              return queue_error;
            }
              
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
