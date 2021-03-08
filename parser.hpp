#pragma once

#include <tao/pegtl.hpp>
#include <tuple>

namespace mq
{
  namespace proto
  {
    enum class connType
    {
      PUBLISHER,
      CONSUMER
    };
    
    using namespace tao::pegtl;
    
    struct publish : pad< string<'P','U','B','L','I','S','H'>, ascii::blank >{};
    struct consume : pad< string<'C','O','N','S','U','M','E'>, ascii::blank >{};
    struct queue : pad< plus<alpha>, ascii::blank>{};
    
    struct connect_req : seq<
      pad< string< 'C','O','N','N','E','C', 'T' >, ascii::blank>,
      sor<
        publish,
        consume
      >,
      queue
    >{};
    
    template< typename Rule >
    struct action
    {};
    
    template<>
    struct action< publish >
    {
      template< class T, class C >
      static void apply(const T&, C& ctx)
      {
        std::get<0>(ctx) = connType::PUBLISHER;
      }
    };
    
    template<>
    struct action< consume >
    {
      template< class T, class C >
      static void apply(const T&, C& ctx)
      {
        std::get<0>(ctx) = connType::CONSUMER;
      }
    };
    
    template<>
    struct action< queue >
    {
      template< class T, class C >
      static void apply(const T& in, C& ctx)
      {
        std::get<1>(ctx) = in.string();
      }
    };
    
    void parse(std::string_view input, std::tuple<connType, std::string>& ctx)
    {
      memory_input<tracking_mode::eager> in{input.data(), ""};
      return parse<connect_req, action>(in, ctx);
    }
    
  } // namespace proto
  
} // namespace mq
