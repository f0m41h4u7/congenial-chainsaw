#pragma once 

#include <boost/asio.hpp>
#include <functional>
#include <set>
#include <string_view>
#include <deque>

#include "Exchange.hpp"
#include "Message.hpp"

namespace mq
{
  using boost::asio::ip::tcp;
  
  class Session;
  
  using handler_t          = std::function<Message&(std::string_view, std::shared_ptr<Session>)>;
  using exchange_deleter_t = std::function<void(const std::string&)>;
  
  enum State { DEFAULT, CONSUMING };
  
  class ISession
  {
  public:
    virtual ~ISession() {}
    virtual void deliver(const Message& msg) = 0;
    virtual State state() const = 0;
    virtual std::string const& queue_name() const = 0;
  };
  
  class SessionStorage
  {
  public:
    constexpr static int max_recent_msgs = 100;
    
    SessionStorage() = default;
    ~SessionStorage() = default;
    
    void add(std::shared_ptr<ISession> s)   { m_sessions.insert(s); }
    void erase(std::shared_ptr<ISession> s) { m_sessions.erase(s); }
    
    void deliver(const Message& msg)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      m_to_write.push_back(msg);
      while (m_to_write.size() > max_recent_msgs)
        m_to_write.pop_front();

      for(auto& s: m_sessions)
      {
        if(s->state() == State::CONSUMING)
        {
          for(auto& m : m_to_write)
          {
            if(s->queue_name() == m.queue_name())
            {
              s->deliver(m);
              m.set_delivered();
            }
          }
        }
      }
      mlock.unlock();
      clean_delivered();
    }
    
    void deliver_previous(std::shared_ptr<ISession> s)
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      for(auto& m : m_to_write)
      {
        if(s->queue_name() == m.queue_name())
        {
          s->deliver(m);
          m.set_delivered();
        }
      }
      mlock.unlock();
      clean_delivered();
    }
    
  private:
    
    void clean_delivered()
    {
      std::unique_lock<std::mutex> mlock(m_mutex);
      for(auto it = m_to_write.begin(); it != m_to_write.end();)
      {
        if((*it).is_delivered())
          it = m_to_write.erase(it);
        else
          ++it;
      }
    }
    
    std::mutex                          m_mutex;
    std::set<std::shared_ptr<ISession>> m_sessions;
    std::deque<Message>                 m_to_write;
  };
  
  class Session : public ISession, public std::enable_shared_from_this<Session>
  {
  public:
    Session(tcp::socket socket, handler_t h, exchange_deleter_t d, SessionStorage& st)
    : m_socket(std::move(socket)),
      m_handler(h),
      m_exch_deleter(d),
      m_sessionStorage(st)
    {}
    
    ~Session()
    {
      if(m_pExchange.use_count() <= 2)
        m_exch_deleter(m_pExchange->name());
    };
    
    void set_exchange(std::shared_ptr<Exchange> exch) { m_pExchange = exch; }
    std::shared_ptr<Exchange> get_exchange() { return m_pExchange; }
    
    State state() const { return m_state; }
    void set_state(State s) { m_state = s; }
    
    std::string const& queue_name() const { return m_pExchange->name(); }
    
    SessionStorage& ref_storage() { return m_sessionStorage; }
    
    void start()
    {
      m_sessionStorage.add(shared_from_this());
      do_read_header();
    }
    
    void deliver(const Message& msg)
    {
      bool write_in_progress = !m_to_write.empty();
      m_to_write.push_back(msg);
      if (!write_in_progress)
        do_write();
    }
    
  private:
    void do_read_header()
    {
      auto self(shared_from_this());
      boost::asio::async_read(m_socket,
          boost::asio::buffer(m_message.data(), Message::header_length),
          [this, self](boost::system::error_code ec, std::size_t)
          {
            if (!ec && m_message.decode_header())
              do_read_body();
            
            else m_sessionStorage.erase(shared_from_this());
          });
    }

    void do_read_body()
    {
      auto self(shared_from_this());
      boost::asio::async_read(m_socket,
          boost::asio::buffer(m_message.body(), m_message.body_length()),
          [this, self](boost::system::error_code ec, std::size_t)
          {
            if (!ec)
            {
              auto res = m_handler(m_message.body(), shared_from_this());
              boost::asio::async_write(m_socket,
                                      boost::asio::buffer(res.data(), res.length()),
                                      [this, self](boost::system::error_code ec, std::size_t)
                                      {
                                        if(ec) m_sessionStorage.erase(shared_from_this());
                                      });
              if(m_state == State::CONSUMING)
                m_sessionStorage.deliver_previous(self);
              
              m_message.clear();
              do_read_header();
            }
            else m_sessionStorage.erase(shared_from_this());
          });
    }
    
    void do_write()
    {
      auto self(shared_from_this());
      boost::asio::async_write(m_socket,
          boost::asio::buffer(m_to_write.front().data(),
            m_to_write.front().length()),
          [this, self](boost::system::error_code ec, std::size_t)
          {
            if(!ec)
            {
              m_to_write.pop_front();
              if(!m_to_write.empty()) do_write();
            }
            else m_sessionStorage.erase(shared_from_this());
          });
    }

    tcp::socket               m_socket;
    Message                   m_message;
    
    handler_t                 m_handler;
    exchange_deleter_t        m_exch_deleter;
    
    std::shared_ptr<Exchange> m_pExchange;
    State                     m_state{State::DEFAULT};
    SessionStorage&           m_sessionStorage;
    
    std::deque<Message>       m_to_write;
  };
  
}//namespace mq
