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
  using boost::asio::ip::tcp;
  
  class Conn;
  
  using handler_t = std::function<std::string(std::string_view, boost::shared_ptr<Conn>)>;
  
  class Conn : public boost::enable_shared_from_this<Conn>
  {
  public:
    using pointer = boost::shared_ptr<Conn>;

    static pointer create(tcp::socket socket, handler_t h) { return pointer(new Conn(std::move(socket), h)); }
    ~Conn() { std::cout << __FUNCTION__ << "\n";};

    void receive() { handle(); }
    
    //void set_exchange(std::shared_ptr<Exchange> exch) { std::cout << __FUNCTION__ << "\n"; m_exch = exch; }
    //std::shared_ptr<Exchange> get_exchange() { std::cout << __FUNCTION__ << "\n"; return m_exch; }
    
    std::shared_ptr<Exchange> m_exch;

  private:
    Conn(tcp::socket socket, handler_t h)
    : m_socket(std::move(socket)),
      m_handler(h)
    {}
    
    void handle()
    {
      auto self(shared_from_this());
      m_socket.async_read_some(boost::asio::buffer(m_data, MAX_PACKET_SIZE),
          [this, self](boost::system::error_code ec, std::size_t length)
          {
            if (!ec)
            {
              auto res = m_handler(std::string{m_data, length}, shared_from_this());
              send(res);
            }
          });
    }
    
    void send(std::string_view sv)
    {
      auto self(shared_from_this());
      boost::asio::async_write(m_socket,
                              boost::asio::buffer(sv.data(), sv.size()),
                              [this, self](boost::system::error_code ec, std::size_t)
                              {
                                if (!ec) handle();
                              });
    }

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

} // namespace mq;
