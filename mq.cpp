#include "mq/router.hpp"

int main()
{
  try
  {
    mq::Router p;
    p.run();
  }
  catch(std::exception& ex) { std::cerr << ex.what() << std::endl; }

  return 0;
}
