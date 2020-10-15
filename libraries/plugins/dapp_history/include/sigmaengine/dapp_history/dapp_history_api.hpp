#pragma once

#include <sigmaengine/app/application.hpp>
#include <sigmaengine/app/sigmaengine_api_objects.hpp>
#include <sigmaengine/app/applied_operation.hpp>

#include <sigmaengine/dapp_history/dapp_history_objects.hpp>

#include <fc/api.hpp>

namespace sigmaengine { namespace dapp_history {
   using namespace sigmaengine::chain;
   using namespace sigmaengine::app;

   namespace detail 
   { 
      class dapp_history_api_impl; 
   }

   class dapp_history_api
   {
      public:
         dapp_history_api( const app::api_context& ctx );
         void on_api_startup();

         /**
          *  dapp operations have sequence numbers from 0 to N where N is the most recent operation. This method
          *  returns operations in the range [from-limit, from]
          *  @param dapp_name - dapp name
          *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
          *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
          */
         map< uint32_t, applied_operation > get_dapp_history( string dapp_name, uint64_t from, uint32_t limit )const;

      private:
         std::shared_ptr< detail::dapp_history_api_impl > _my;
   };

} } //namespace sigmaengine::token

FC_API( sigmaengine::dapp_history::dapp_history_api,
   ( get_dapp_history )
)