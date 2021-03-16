#pragma once 

#include "rapidjson/document.h"
#include <string_view>

namespace mq
{
  const char* method_str("method");
  const char* queue_str ("queue");
  const char* data_str  ("data");
  
  const std::string method_connect = "CONNECT";
  const std::string method_publish = "PUBLISH";
  const std::string method_consume = "CONSUME";
  
  enum class Method
  {
    CONNECT,
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
      if (m_parser.Parse(req_sv.data()).HasParseError())
        return ErrorCode::ERROR;

      if(!m_parser.HasMember(method_str))
        return ErrorCode::ERROR;

      auto method = m_parser[method_str].GetString();
      
      if(method == method_connect)
      {
        if(!m_parser.HasMember(queue_str))
          return ErrorCode::ERROR;
        
        m_method = Method::CONNECT;
        m_queue = m_parser[queue_str].GetString();
        return ErrorCode::OK;
      }

      if(method == method_publish)
      {
        if(!m_parser.HasMember(data_str))
          return ErrorCode::ERROR;
        m_method = Method::PUBLISH;
        m_data = m_parser[data_str].GetString();
        return ErrorCode::OK;
      }
      
      if(method == method_consume)
      {
        m_method = Method::CONSUME;
        return ErrorCode::OK;
      }
      
      return ErrorCode::ERROR;
    }
    
    Method      m_method;
    std::string m_queue;
    std::string m_data;
    
  private:
    rapidjson::Document m_parser;
  };
  
} //namespace mq
