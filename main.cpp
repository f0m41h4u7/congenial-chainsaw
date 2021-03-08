#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "mq.hpp"

bool done = false;

void handler(int)
{
  std::cout << "SIGINT\n";
  done = true;
}

int main()
{
  try
  {
    //signal(SIGINT, handler);
    mq::MQ<std::string> p;
    p.run();
    
  /*  auto publisher = [&](){
      std::string line;
      std::mutex mx;
      std::cout << "[publisher " << std::this_thread::get_id() << "]: start\n";
      do
      {
        std::unique_lock<std::mutex> lock(mx);
        if(done) { lock.unlock(); return; }
        lock.unlock();
        std::cin >> line;
        if(!line.empty())
        {
          std::cout << "[publisher " << std::this_thread::get_id() <<"]: publish " << line << std::endl;
          lock.lock();
          p->publish(line);
          lock.unlock();
        }
      }
      while(true);
    };*/
    
   /* auto consumer = [&](){
      std::string res;
      std::mutex mx;
      std::cout << "[consumer " << std::this_thread::get_id() <<"]: start\n";
      do
      {
        std::unique_lock<std::mutex> lock(mx);
        if(done) { lock.unlock(); return; }
        res = p->receive();
        lock.unlock();
        if(!res.empty()) std::cout << "[consumer " << std::this_thread::get_id() <<"]: receive " << res << std::endl;
      }
      while(true);
    };
    
    auto pub1 = std::thread(publisher);
    auto pub2 = std::thread(publisher);
    auto con1 = std::thread(consumer);
    auto con2 = std::thread(consumer);
    
    while(true)
    {
      if(done)
      {
        con1.join();
        std::cout << "[consumer 1]: stop\n";
        con2.join();
        std::cout << "[consumer 2]: stop\n";
        return 0;
      }
    }*/
  }
  catch(std::exception& ex) { std::cerr << ex.what() << std::endl; }

  return 0;
}
