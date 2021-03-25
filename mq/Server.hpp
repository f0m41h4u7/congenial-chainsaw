#pragma once

#include "Session.hpp"

namespace mq 
{
  class Server
  {
  public:
    Server(boost::asio::io_context& io_context,
      const tcp::endpoint& endpoint, handler_t h, exchange_deleter_t d)
    : m_acceptor(io_context, endpoint),
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
            std::make_shared<Session>(std::move(socket), m_handler, m_exch_deleter, m_sessionStorage)->start();
          else
            std::cerr << ec << std::endl;

          listenAndServe();
        });
    }
  
  private:
    tcp::acceptor      m_acceptor;
    
    handler_t          m_handler;
    exchange_deleter_t m_exch_deleter;
    
    SessionStorage     m_sessionStorage;
  };

} // namespace mq;
