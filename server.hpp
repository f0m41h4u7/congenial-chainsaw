#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>
#include <string>
#include <string_view>

#define MAX_PACKET_SIZE 4096

namespace mq 
{
  namespace net
  {
    using boost::asio::ip::tcp;
    
    using handler_t = std::function<std::string_view(std::string_view)>;
    
    class Conn : public boost::enable_shared_from_this<Conn>
    {
    public:
      using pointer = boost::shared_ptr<Conn>;

      static pointer create(tcp::socket socket, handler_t h) { return pointer(new Conn(std::move(socket), h)); }
      ~Conn() = default;

      void receive() { handle(); }

    private:
      Conn(tcp::socket socket, handler_t h)
      : m_socket(std::move(socket)),
        m_handler(h)
      {}
      
      void handle();
      void send(std::string_view);

      tcp::socket m_socket;
      char        m_data[MAX_PACKET_SIZE];
      handler_t   m_handler;
    };
      
    class Server
    {
    public:
      Server(short unsigned int port, boost::asio::io_service& svc, handler_t handler)
      : m_acceptor(svc, tcp::endpoint(tcp::v4(), port)),
        m_handler(handler)
      {
        listenAndServe();
      }
      
      ~Server() = default;
      
      void listenAndServe()
      {
        m_acceptor.async_accept(
          [this](boost::system::error_code ec, tcp::socket socket)
          {
            if (!ec)
            {
              auto conn = Conn::create(std::move(socket), m_handler);
              conn->receive();
            }
            listenAndServe();
          }
        );
      }
    
    private:
      boost::asio::io_service m_svc;
      tcp::acceptor           m_acceptor;
      handler_t               m_handler;
    };
  } // namespace net;

} // namespace mq;
