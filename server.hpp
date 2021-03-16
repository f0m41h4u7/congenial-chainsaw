#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <string_view>

namespace mq 
{
  using boost::asio::ip::tcp;
  
  const short unsigned int MAX_PACKET_SIZE = 4096;
  
  class Conn;
  
  using handler_t          = std::function<std::string(std::string_view, std::shared_ptr<Conn>)>;
  using exchange_deleter_t = std::function<void(const std::string&)>;
  
  class Conn : public std::enable_shared_from_this<Conn>
  {
  public:
    using pointer = std::shared_ptr<Conn>;

    static pointer create(tcp::socket socket, handler_t h, exchange_deleter_t d) { return pointer(new Conn(std::move(socket), h, d)); }
    ~Conn()
    {
      std::cout << __FUNCTION__ << " " << m_exch.use_count() << "\n";
      if(m_exch.use_count() <= 2)
        m_exch_deleter(m_exch->name());
    };

    void receive() { handle(); }
    
    void set_exchange(std::shared_ptr<Exchange> exch) { m_exch = exch; }
    std::shared_ptr<Exchange> get_exchange() { return m_exch; }

  private:
    Conn(tcp::socket socket, handler_t h, exchange_deleter_t d)
    : m_socket(std::move(socket)),
      m_handler(h),
      m_exch_deleter(d)
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

    tcp::socket               m_socket;
    char                      m_data[MAX_PACKET_SIZE];
    handler_t                 m_handler;
    exchange_deleter_t        m_exch_deleter;
    std::shared_ptr<Exchange> m_exch;
  };
    
  class Server
  {
  public:
    Server(short unsigned int port, boost::asio::io_service& svc, handler_t h, exchange_deleter_t d)
    : m_acceptor(svc, tcp::endpoint(tcp::v4(), port)),
      m_handler(h),
      m_exch_deleter(d)
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
            auto conn = Conn::create(std::move(socket), m_handler, m_exch_deleter);
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
    exchange_deleter_t      m_exch_deleter;
  };

} // namespace mq;
