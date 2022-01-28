#pragma once
#include <fc/uint128.hpp>

#include <sigmaengine/chain/sigmaengine_object_types.hpp>

#include <sigmaengine/protocol/asset.hpp>

namespace sigmaengine { namespace chain {

   using sigmaengine::protocol::asset;
   using sigmaengine::protocol::price;

   /**
    * @class dynamic_global_property_object
    * @brief Maintains global state information
    * @ingroup object
    * @ingroup implementation
    *
    * This is an implementation detail. The values here are calculated during normal chain operations and reflect the
    * current values of global blockchain properties.
    */
   class dynamic_global_property_object : public object< dynamic_global_property_object_type, dynamic_global_property_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         dynamic_global_property_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         dynamic_global_property_object(){}

         id_type           id;

         uint32_t          head_block_number = 0;
         block_id_type     head_block_id;
         time_point_sec    time;
         account_name_type current_bobserver;

         asset             current_supply              = asset( 0, SGT_SYMBOL );   /// total tnc supply

         /**
          *  Maximum block size is decided by the set of active bobservers which change every round.
          *  Each bobserver posts what they think the maximum size should be as part of their bobserver
          *  properties, the median size is chosen to be the maximum block size for the round.
          *
          *  @note the minimum value for maximum_block_size is defined by the protocol to prevent the
          *  network from getting stuck by bobservers attempting to set this too low.
          */
         uint32_t          maximum_block_size = 0;

         /**
          * The current absolute slot number.  Equal to the total
          * number of slots since genesis.  Also equal to the total
          * number of missed slots plus head_block_number.
          */
         uint64_t          current_aslot = 0;

         /**
          * used to compute bobserver participation.
          */
         fc::uint128_t     recent_slots_filled;
         uint8_t           participation_count = 0; ///< Divide by 128 to compute participation percentage

         asset             dapp_transaction_fee = SIGMAENGINE_DAPP_TRANSACTION_FEE;

         uint32_t          last_irreversible_block_num = 0;
         uint32_t          current_bproducer_count = 0;


         time_point_sec    next_update_bproducer_owner_time;
         time_point_sec    last_dapp_voting_aggregation_time;

         uint32_t          total_transfer = 0;
   };

   typedef multi_index_container<
      dynamic_global_property_object,
      indexed_by<
         ordered_unique< tag< by_id >,
            member< dynamic_global_property_object, dynamic_global_property_object::id_type, &dynamic_global_property_object::id > >
      >,
      allocator< dynamic_global_property_object >
   > dynamic_global_property_index;

} } // sigmaengine::chain

FC_REFLECT( sigmaengine::chain::dynamic_global_property_object,
             (id)
             (head_block_number)
             (head_block_id)
             (time)
             (current_bobserver)
             (current_supply)
             (maximum_block_size)
             (current_aslot)
             (recent_slots_filled)
             (participation_count)
             (dapp_transaction_fee)
             (last_irreversible_block_num)
             (current_bproducer_count)
             (next_update_bproducer_owner_time)
             (last_dapp_voting_aggregation_time)
             (total_transfer)
          )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::dynamic_global_property_object, sigmaengine::chain::dynamic_global_property_index )
