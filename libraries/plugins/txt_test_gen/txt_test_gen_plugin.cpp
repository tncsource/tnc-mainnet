/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <sigmaengine/txt_test_gen/txt_test_gen_api.hpp>
#include <sigmaengine/txt_test_gen/txt_test_gen_plugin.hpp>
#include <sigmaengine/protocol/types.hpp>

#include <sigmaengine/chain/database.hpp>
#include <sigmaengine/app/api.hpp>

#include <fc/api.hpp>
#include <fc/smart_ref_impl.hpp>
#include <graphene/utilities/key_conversion.hpp>

namespace sigmaengine { namespace txt_test_gen {
namespace bpo = boost::program_options;

namespace detail {
   struct txt_test_gen_plugin_impl {
      std::shared_ptr<boost::asio::io_service>             gen_ios; 
      optional<boost::asio::io_service::work*>             gen_ios_work; 
      uint16_t                                             thread_pool_size; 
      optional<std::vector<boost::thread*>>                thread_pool; 
      std::shared_ptr<boost::asio::high_resolution_timer>  timer; 
      account_name_type                                    newaccountA; 
      account_name_type                                    newaccountB; 

      uint64_t nonce_prefix;
      bool     running{false};
      unsigned timer_timeout;
      uint16_t tr_count;
      fc::optional<fc::ecc::private_key> init_account_priv_key;
   };
}

txt_test_gen_plugin::txt_test_gen_plugin( application* app )
   : plugin( app ), my(new detail::txt_test_gen_plugin_impl)
{}

txt_test_gen_plugin::~txt_test_gen_plugin()
{}

void txt_test_gen_plugin::arm_timer(boost::asio::high_resolution_timer::time_point s) { 
   my->timer->expires_at(s + std::chrono::milliseconds(my->timer_timeout)); 
   my->gen_ios->post( [this]() { 
      send_transaction([this](const fc::exception_ptr& e){ 
         if (e) { 
            elog("pushing transaction failed: ${e}", ("e", e->to_detail_string())); 
            if(my->running) 
               stop_generation(); 
         } 
      }, my->nonce_prefix++); 
   }); 
   my->timer->async_wait([this](const boost::system::error_code& ec) { 
      if(!my->running || ec) 
         return; 
      arm_timer(my->timer->expires_at()); 
   }); 
} 

void txt_test_gen_plugin::send_transaction(std::function<void(const fc::exception_ptr&)> next, uint64_t nonce_prefix) {
   try {
      chain::database& db = database();
      signed_transaction trx;
      transfer_operation t;
      for (int i = 0 ; i < my->tr_count ; i++)     
      {  
         t.from = my->newaccountA;
         t.to = my->newaccountB;
         t.amount = asset( nonce_prefix * 10000 + i + 1,SGT_SYMBOL);
         trx.operations.push_back(t);
      }

      ilog("send ${c} transactions", ("c",trx.operations.size()));

      trx.set_expiration( db.head_block_time() + 60 );
      trx.sign( *my->init_account_priv_key, db.get_chain_id() );

      //app().chain_database()->push_transaction(trx);
      app().p2p_node()->broadcast_transaction(trx);
   } catch ( const fc::exception& e ) {
      next(e.dynamic_copy_exception());
   }
}

void txt_test_gen_plugin::start_generation(uint16_t thread, unsigned repeat, uint16_t count, string accountA, string accountB, string key)
{
   ilog( "start txt_test_gen plugin !!" );

   if(my->running) 
      throw fc::exception(fc::invalid_operation_exception_code);

   FC_ASSERT( thread > 0 ); 
   FC_ASSERT( repeat > 0 );
   FC_ASSERT( count > 0 );
   FC_ASSERT( accountA.size() > 0);
   FC_ASSERT( accountB.size() > 0);
   FC_ASSERT( key.size() > 0);

   my->thread_pool_size = thread;
   my->timer_timeout = repeat;
   my->tr_count = count;
   my->newaccountA = accountA;
   my->newaccountB = accountB;
   my->init_account_priv_key = graphene::utilities::wif_to_key(key);

   my->running = true;
   my->nonce_prefix = 0;
   my->gen_ios = std::make_shared<boost::asio::io_service>();
   my->gen_ios_work = new boost::asio::io_service::work(*my->gen_ios);

   for( int i = 0; i < my->thread_pool_size; ++i ) {
      my->thread_pool->push_back( new boost::thread( [ios = my->gen_ios]() { ios->run(); }) );
   }
   my->timer = std::make_shared<boost::asio::high_resolution_timer>(*my->gen_ios);

   my->gen_ios->post( [this]() { 
      arm_timer(boost::asio::high_resolution_timer::clock_type::now()); 
   }); 
}

void txt_test_gen_plugin::stop_generation() 
{
   if(!my->running)
      throw fc::exception(fc::invalid_operation_exception_code);

   my->timer->cancel();
   my->running = false;
   delete *my->gen_ios_work;
   if( my->gen_ios )
      my->gen_ios->stop();
   for( auto thread : *my->thread_pool ) {
      thread->join();
   }
   ilog("Stopping transaction generation test");
}

void txt_test_gen_plugin::plugin_set_program_options(bpo::options_description& cli, bpo::options_description& cfg)
{
   /*cli.add_options()
         ("thread-size", boost::program_options::value<std::uint16_t>()->default_value(1), "test node thread size")
         ("timer-timeout", boost::program_options::value<unsigned>()->default_value(40), "transaction repeat time")
         ("transaction-count", boost::program_options::value<std::uint16_t>()->default_value(10), "transaction count one time")
         ("newaccountA", boost::program_options::value<std::string>()->composing(), "trasfer account")
         ("newaccountB", boost::program_options::value<std::string>()->composing(), "receiver account")
         ("init-account-priv-key", boost::program_options::value<std::string>()->composing(), "trasfer account private key")
         ;
   cfg.add(cli);*/
}

void txt_test_gen_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
   ilog( "Intializing txt_test_gen plugin" );

   /*FC_ASSERT( options.count( "thread-size" ) > 0 ); 
   FC_ASSERT( options.count( "timer-timeout" ) > 0 );

   my->thread_pool_size = options.at("thread-size").as<std::uint16_t>();
   my->timer_timeout = options.at("timer-timeout").as<unsigned>();
   my->tr_count = options.at("transaction-count").as<uint16_t>();
   my->newaccountA = options.at("newaccountA").as<std::string>();
   my->newaccountB = options.at("newaccountB").as<std::string>();
   my->init_account_priv_key = graphene::utilities::wif_to_key(options.at("init-account-priv-key").as<std::string>());*/
}

void txt_test_gen_plugin::plugin_startup()
{
   app().register_api_factory< txt_test_gen_api >( "txt_test_gen_api" );
}

void txt_test_gen_plugin::plugin_shutdown()
{
   stop_generation();
}

} }

SIGMAENGINE_DEFINE_PLUGIN( txt_test_gen, sigmaengine::txt_test_gen::txt_test_gen_plugin )
