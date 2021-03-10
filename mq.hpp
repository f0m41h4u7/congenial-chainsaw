#pragma once

#include "rapidjson/document.h"

#include "queue.hpp"
#include "server.cpp"

#define DEFAULT_PORT 31337

namespace mq
{
  constexpr std::string_view ok_response = "OK\n";
  constexpr std::string_view parse_error = "Error: failed to parse request\n";
  
  constexpr std::string_view method_str = "method";
  constexpr std::string_view queue_str = "queue";
  
  enum class Method
  {
    QUEUE_DECLARE,
    QUEUE_PURGE,
    PUBLISH,
    CONSUME
  };
  
  class router
  {
  public:
    router() = default;
    ~router() = default;
    
    void run()
    {
      boost::asio::io_service svc;
      
      net::Server s(
        DEFAULT_PORT,
        svc,
        [&](std::string_view req) -> std::string
        {
          //publish(std::forward<std::string>(req.data()));
          return parse_request(req).data();
          
          //std::cout << "method: " << m_parser["method"].GetString() << "\n";
        }
      );
      
      svc.run();
    }
    
    std::string_view parse_request(std::string_view req)
    {
      if (m_parser.Parse(req.data()).HasParseError())
        return parse_error;
      
      if(!m_parser.HasMember(method_str.data()) || !m_parser.HasMember(queue_str.data()))
        return parse_error;
      
      return ok_response;
    }
    
    void publish(std::string&& data)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      m_queue.push(std::forward<std::string>(data));
    }
    
    std::string receive()
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      return m_queue.pop();
    }
  
  private:
    std::mutex m_mutex;
    cqueue<std::string> m_queue;
    rapidjson::Document m_parser;
  };

} //namespace mq
