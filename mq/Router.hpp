#pragma once

#include <map>

#include "Server.hpp"
#include "Request.hpp"

namespace mq
{
  const short unsigned int DEFAULT_PORT = 31337;
  
  constexpr std::string_view ok_response   = "OK\n";
  constexpr std::string_view parse_error   = "Error: failed to parse request\n";
  constexpr std::string_view queue_error   = "Error: wrong queue name\n";
  constexpr std::string_view default_error = "Error\n";
  
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
    
    bool exchange_exists(std::string& q_name)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      if(m_exchanges.contains(q_name))
        return true;
      return false;
    }
    
    void run()
    {
      boost::asio::io_context ctx;
      tcp::endpoint endpoint(tcp::v4(), DEFAULT_PORT);
      
      Server s(
        ctx,
        endpoint,
        [&](std::string_view data, std::shared_ptr<Session> session) -> Message&
        {
          Request req;
          auto err = req.parseAndValidate(data);
          if(err != ErrorCode::OK)
            return m_parse_err_msg;
          
          switch(req.m_method)
          {
            case Method::CONNECT:
            {
              session->set_exchange(queue_connect(req.m_queue));
              return m_OK_msg;
            }
            
            case Method::PUBLISH:
            {
              auto q_name = session->get_exchange()->name();
              if(exchange_exists(q_name))
              {
                session->get_exchange()->publish(req.m_data);
                session->ref_storage().deliver(Message{req.m_data, q_name});
                return m_OK_msg;
              }
              return m_queue_err_msg;
            }
            
            case Method::CONSUME:
            {
              auto q_name = session->get_exchange()->name();
              if(exchange_exists(q_name))
              {
                session->set_state(State::CONSUMING);
                return m_OK_msg;
              }
              return m_queue_err_msg;
            }
            
            default:
              return m_err_msg;
          }
        },
        [&](const std::string& name)
        {
          std::unique_lock<std::mutex> mlock(m_mutex);
          m_exchanges.erase(name);
        }
      );
      
      std::cout << "Server started at " << endpoint << std::endl;
      
      ctx.run();
    }
  
  private:
    std::mutex                                       m_mutex;
    QueueStorage<3>                                  m_queueStorage;
    std::map<std::string, std::shared_ptr<Exchange>> m_exchanges;
    
    Message m_OK_msg{ok_response};
    Message m_parse_err_msg{parse_error};
    Message m_queue_err_msg{queue_error};
    Message m_err_msg{default_error};
  };

} //namespace mq
