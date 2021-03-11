#pragma once

#include <map>
#include <tao/json.hpp>

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
  
  const std::map<std::string, Method> g_methods;
  
  struct Request
  {
    std::string method;
    std::string queue;
    std::string_view response;
 //   std::string data;
  };
  
  template<>
  struct my_traits< Request >
  {
    template< template< typename... > class Traits >
    static void to( const tao::json::basic_value< Traits >& v, Request& d )
    {
        const auto& object = v.get_object();
        d.method = v.at( method_str ).template as< std::string >();
        d.queue = v.at( queue_str ).template as< std::string >();
    }

    template< template< typename... > class Traits >
    static Request as( const tao::json::basic_value< Traits >& v )
    {
        Request result;
        const auto& object = v.get_object();
        result.method = v.at( method_str ).template as< std::string >();
        result.queue = v.at( queue_str ).template as< std::string >();
        return result;
    }
    
    template< template< typename... > class Traits >
    static void assign( tao::json::basic_value< Traits >& v, const Request& r )
    {
        v = {
          { method_str, r.method },
          { queue_str, r.queue }//,
          //{ data_str, r.data }
        };
    }
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
        [&](std::string_view req) -> std::string_view
        {
          //publish(std::forward<std::string>(req.data()));
          return parseAndValidate(req).response;
          
          //std::cout << "method: " << m_parser["method"].GetString() << "\n";
        }
      );
      
      svc.run();
    }

    Request parseAndValidate(std::string_view req_data) const
    {
      Request req;
      try
      {
        auto v = tao::json::from_string(req_data);
        Request req = v.as< Request >();
        req.response = ok_response;
      }
      catch (std::exception& ex)
      {
        req.response = parse_error;
        return req;
      }
      
  /*    if (m_parser.Parse(req.data()).HasParseError())
        return parse_error;
      
      if(!m_parser.HasMember(method_str.data()) || !m_parser.HasMember(queue_str.data()))
        return format_error;
      
      if(m_parser[])*/
      
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
   // rapidjson::Document m_parser;
  };

} //namespace mq
