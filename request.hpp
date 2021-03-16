#pragma once 

#include "rapidjson/document.h"
#include <string_view>

namespace mq
{
  const char* method_str("method");
  const char* queue_str("queue");
  const char* data_str("data");
  
  const std::string method_queue_connect = "QUEUE_CONNECT";
  const std::string method_publish = "PUBLISH";
  const std::string method_consume = "CONSUME";
  
  enum class Method
  {
    QUEUE_CONNECT,
    PUBLISH,
    CONSUME
  };
  
  enum ErrorCode { OK, ERROR };
  
  struct Request
  {
    Request() = default;
    ~Request() = default;
    
    ErrorCode parseAndValidate(std::string_view req_sv)
    {
      //std::cout << __FUNCTION__ << "\n";
      if (m_parser.Parse(req_sv.data()).HasParseError())
        return ErrorCode::ERROR;
      if(!m_parser.HasMember(method_str) || !m_parser.HasMember(queue_str))
        return ErrorCode::ERROR;
      
      auto method = m_parser[method_str].GetString();
      
      if(method == method_queue_connect)
      {
        //std::cout << "method connect\n";
        m_method = Method::QUEUE_CONNECT;
      }
      
      /*else if(method == method_publish)
      {
        std::cout << "method publish\n";
        if(!m_parser.HasMember(data_str))
          return ErrorCode::ERROR;
        std::cout << "method publish no error\n";
        m_method = Method::PUBLISH;
        m_data = m_parser[data_str].GetString();
      }
      
      else if(method == method_consume)
          m_method = Method::CONSUME;*/
      
      else return ErrorCode::ERROR;
      
      m_queue = m_parser[queue_str].GetString();
      return ErrorCode::OK;
    }
    
    
    Method m_method;
    std::string m_queue;
    std::string m_data;
    ErrorCode m_err{ErrorCode::OK};
    
  private:
    rapidjson::Document m_parser;
  };
  
} //namespace mq
