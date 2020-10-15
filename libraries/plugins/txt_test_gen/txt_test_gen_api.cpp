#include <sigmaengine/txt_test_gen/txt_test_gen_api.hpp>

namespace sigmaengine { namespace txt_test_gen {

namespace detail
{
   class txt_test_gen_api_impl
   {
      public:
         txt_test_gen_api_impl( sigmaengine::app::application& app )
            :_app( app ) {}

         void start_generation(uint16_t thread, unsigned repeat, uint16_t count, string accountA, string accountB, string key)const;
         void stop_generation()const;
         sigmaengine::app::application& _app;
   };

   void txt_test_gen_api_impl::start_generation(uint16_t thread, unsigned repeat, uint16_t count, string accountA, string accountB, string key)const
   {
      _app.get_plugin< txt_test_gen_plugin >( TXT_TEXT_GEN_PLUGIN_NAME )->start_generation(thread, repeat, count, accountA, accountB, key);
   }
   
   void txt_test_gen_api_impl::stop_generation()const
   {
      _app.get_plugin< txt_test_gen_plugin >( TXT_TEXT_GEN_PLUGIN_NAME )->stop_generation();
   }

} // detail

txt_test_gen_api::txt_test_gen_api( const sigmaengine::app::api_context& ctx )
{
   my = std::make_shared< detail::txt_test_gen_api_impl >( ctx.app );
}

void txt_test_gen_api::on_api_startup() {}

void txt_test_gen_api::start_generation(uint16_t thread, unsigned repeat, uint16_t count, string accountA, string accountB, string key)const
{
   my->start_generation(thread, repeat, count, accountA, accountB, key);
}

void txt_test_gen_api::stop_generation()const
{
   my->stop_generation();
}

} } // sigmaengine::txt_test_gen
