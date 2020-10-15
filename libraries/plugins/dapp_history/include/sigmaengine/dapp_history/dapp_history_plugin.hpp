#pragma once

#include <sigmaengine/app/plugin.hpp>
#include <sigmaengine/chain/database.hpp>

#include <fc/thread/future.hpp>

#define DAPP_HISTORY_PLUGIN_NAME "dapp_history"

namespace sigmaengine { namespace dapp_history {
   using sigmaengine::app::application;
   using namespace chain;
   using namespace sigmaengine::protocol;

   namespace detail { class dapp_history_plugin_impl; }

   class dapp_history_plugin : public sigmaengine::app::plugin
   {
      public:
         dapp_history_plugin( application* app );

         std::string plugin_name()const override { return DAPP_HISTORY_PLUGIN_NAME; }
         virtual void plugin_initialize(const boost::program_options::variables_map& options) override;
         virtual void plugin_startup() override;

         friend class detail::dapp_history_plugin_impl;
         
      private:
         std::unique_ptr<detail::dapp_history_plugin_impl> _my;
   };

} } //namespace sigmaengine::dapp_history