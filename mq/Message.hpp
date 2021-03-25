#pragma once

#include <string_view>

namespace mq 
{
  class Message
  {
  public:
    constexpr static int header_length = 4;
    constexpr static int max_body_length = 1024;

    Message() = default;
    ~Message() = default;
    
    Message(std::string_view sv) { set_message(sv); }
    Message(std::string_view sv, std::string& q_name)
    : m_queue_name(q_name)
    { set_message(sv); }

    const char* data() const { return m_data; }
    char* data() { return m_data; }

    std::size_t length() const { return header_length + m_body_length; }

    const char* body() const { return m_data + header_length; }
    char* body() { return m_data + header_length; }

    std::size_t body_length() const { return m_body_length;}
    
    void clear() { memset(&m_data[0], 0, sizeof(m_data)); }
    
    void set_delivered() { m_is_delivered = true; }
    bool is_delivered() const { return m_is_delivered; }
    
    std::string const& queue_name() { return m_queue_name; }
    void queue_name(std::string& name) { m_queue_name = name; }
    
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
    bool        m_is_delivered{false};
    std::string m_queue_name;
  };
} // namespace mq;
