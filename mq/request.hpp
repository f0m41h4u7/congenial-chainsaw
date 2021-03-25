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

  class Message
  {
  public:
    constexpr static int header_length = 4;
    constexpr static int max_body_length = 1024;

    Message() = default;
    ~Message() = default;
    
    Message(std::string_view sv) { set_message(sv); }

    const char* data() const { return m_data; }
    char* data() { return m_data; }

    std::size_t length() const { return header_length + m_body_length; }

    const char* body() const { return m_data + header_length; }
    char* body() { return m_data + header_length; }

    std::size_t body_length() const { return m_body_length;}
    
    void clear() { memset(&m_data[0], 0, sizeof(m_data)); }
    
    void set_message(std::string_view sv)
    {
      if(sv.size() > max_body_length) return;
      clear();
      std::sprintf(&m_data[0], "%4d", static_cast<int>(sv.size()));
      std::memcpy(&m_data[4], sv.data(), sv.size());
      m_body_length = sv.size();
    }

    void body_length(std::size_t new_length)
    {
      m_body_length = new_length;
      if (m_body_length > max_body_length)
        m_body_length = max_body_length;
    }

    bool decode_header()
    {
      char header[header_length + 1] = "";
      std::strncat(header, m_data, header_length);
      m_body_length = std::atoi(header);
      if (m_body_length > max_body_length)
      {
        m_body_length = 0;
        return false;
      }
      return true;
    }

  private:
    char        m_data[header_length + max_body_length];
    std::size_t m_body_length{0};
  };
  
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
      
      if(method == method_consume)
      {
        m_method = Method::CONSUME;
        return ErrorCode::OK;
      }
      
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

      return ErrorCode::ERROR;
    }
    
    Method      m_method;
    std::string m_queue;
    std::string m_data;
    
  private:
    rapidjson::Document m_parser;
  };
  
} //namespace mq
