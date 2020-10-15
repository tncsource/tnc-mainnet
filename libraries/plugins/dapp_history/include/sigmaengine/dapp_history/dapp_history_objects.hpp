#pragma once

#include <sigmaengine/app/plugin.hpp>

#include <sigmaengine/chain/sigmaengine_object_types.hpp>
#include <sigmaengine/protocol/types.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace sigmaengine { namespace dapp_history {
   using namespace std;
   using namespace sigmaengine::chain;
   using namespace boost::multi_index;
   using namespace sigmaengine::protocol;

   enum dapp_history_by_key_object_type
   {
      dapp_history_object_type              = (DAPP_HISTORY_SPACE_ID << 8)
   };

   class dapp_history_object : public object< dapp_history_object_type, dapp_history_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         dapp_history_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         id_type                 id;

         dapp_name_type          dapp_name;
         uint32_t                sequence = 0;
         operation_id_type       op;
   };

   typedef oid< dapp_history_object > dapp_history_id_type;

   struct by_id;
   struct by_dapp_name;

   typedef multi_index_container<
      dapp_history_object,
      indexed_by<
         ordered_unique< tag< by_id >, 
            member< dapp_history_object, dapp_history_id_type, &dapp_history_object::id > 
         >,
         ordered_unique< tag< by_dapp_name >,
            composite_key< dapp_history_object,
               member< dapp_history_object, dapp_name_type, &dapp_history_object::dapp_name >,
               member< dapp_history_object, uint32_t, &dapp_history_object::sequence >
            >,
            composite_key_compare< std::less< dapp_name_type >, std::greater< uint32_t > >
         >
      >,
      allocator< dapp_history_object >
   > dapp_history_index;

} } //namespace sigmaengine::dapp_history

FC_REFLECT( sigmaengine::dapp_history::dapp_history_object, (id)(dapp_name)(sequence)(op) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::dapp_history::dapp_history_object, sigmaengine::dapp_history::dapp_history_index )

