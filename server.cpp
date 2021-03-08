#include "server.hpp" 

namespace mq
{
  namespace net
  {
    void Conn::handle()
    {
      auto self(shared_from_this());
      m_socket.async_read_some(boost::asio::buffer(m_data, MAX_PACKET_SIZE),
          [this, self](boost::system::error_code ec, std::size_t length)
          {
            if (!ec)
            {
              auto res = m_handler(std::string{m_data, length});
              if(proto::parse(res))
              {
                //connect request;
              }
              else
              {
                // check session; process data;
              }
              send(OK_RESPONSE, OK_RESPONSE_SZ);
            }
          });
    }
    
    void Conn::send(std::string_view sv, std::size_t length)
    {
      auto self(shared_from_this());
      boost::asio::async_write(m_socket,
                              boost::asio::buffer(sv.data(), length),
                              [this, self](boost::system::error_code ec, std::size_t)
                              {
                                if (!ec) handle();
                              });
    }
      
  } //namespace net
  
}//namespace mq
