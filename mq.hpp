#pragma once

#include <map>
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
  constexpr std::string_view data_str = "data";
  
  enum class Method
  {
    QUEUE_DECLARE,
    QUEUE_PURGE,
    PUBLISH,
    CONSUME
  };
  
  //const std::map<std::string, Method> g_methods;
  
  struct Request
  {
    Request() = default;
    ~Request() = default;
    
    std::string method;
    std::string queue;
    std::string_view response;
 //   std::string data;
  };
  
  class Router
  {
  public:
    Router() = default;
    ~Router() = default;
    
    void run()
    {
      boost::asio::io_service svc;
      
      net::Server s(
        DEFAULT_PORT,
        svc,
        [&](std::string_view req) -> std::string
        {
          return parseAndValidate(req).method;
        }
      );
      
      svc.run();
    }

    Request parseAndValidate(std::string_view req_sv)
    {
      Request req;
      if (m_parser.Parse(req_sv.data()).HasParseError())
      {
        req.response = parse_error;
        return req;
      }
      
      if(!m_parser.HasMember(method_str.data()) || !m_parser.HasMember(queue_str.data()))
      {
        req.response = parse_error;
        return req;
      }
      
      req.method = m_parser[method_str.data()].GetString();
      req.queue = m_parser[queue_str.data()].GetString();
      req.response = ok_response;
      return req;
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
