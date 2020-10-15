#pragma once
#include <sigmaengine/txt_test_gen/txt_test_gen_plugin.hpp>
#include <fc/api.hpp>

namespace sigmaengine { namespace app {
   struct api_context;
} }

namespace sigmaengine { namespace txt_test_gen {

namespace detail
{
   class txt_test_gen_api_impl;
}

class txt_test_gen_api
{
   public:
      txt_test_gen_api( const sigmaengine::app::api_context& ctx );
      void on_api_startup();
      void start_generation(uint16_t thread, unsigned repeat, uint16_t count, string accountA, string accountB, string key)const;
      void stop_generation()const;

   private:
      std::shared_ptr< detail::txt_test_gen_api_impl > my;
};

} } // sigmaengine::txt_test_gen

FC_API( sigmaengine::txt_test_gen::txt_test_gen_api,
(start_generation)
(stop_generation)
)
