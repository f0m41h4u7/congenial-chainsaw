#pragma once

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
      std::cout << __FUNCTION__ << " " << m_exchanges.size() << "\n";
      
      std::unique_lock<std::mutex> mlock(m_mutex);
      if(m_exchanges.find(name) != m_exchanges.end())
      {
        std::cout << "EXISTS\n";
        auto exch = m_exchanges.at(name);
        mlock.unlock();
        return exch;
      }
      auto exch = std::make_shared<Exchange>(name, std::move(m_queueStorage.acquire_queue()));
      m_exchanges[name] = exch;
      return exch;
    }
    
    void run()
    {
      boost::asio::io_service svc;
      
      Server s(
        DEFAULT_PORT,
        svc,
        [&](std::string_view data, boost::shared_ptr<Conn> conn) -> std::string
        {
          Request req;
          auto err = req.parseAndValidate(data);
          if(err == ErrorCode::ERROR)
            return parse_error.data();
          
          switch(req.m_method)
          {
            case Method::QUEUE_CONNECT:
            {
              conn->m_exch = queue_connect(req.m_queue);
              return ok_response.data();
            }
            /*case Method::PUBLISH:
            {
              if(conn->m_exch->name() == req.m_queue)
              //conn->get_exchange()->publish(req.m_data);
              std::cout << "published\n";
              return ok_response.data();
            }*/
            default:
              return ok_response.data();
          }
        }
      );
      
      svc.run();
    }
  
  private:
    std::mutex m_mutex;
    QueueStorage<5> m_queueStorage;
    //std::map<std::string, std::shared_ptr<Exchange>> m_exchanges;
    exchanges_map_t m_exchanges;
  };

} //namespace mq
