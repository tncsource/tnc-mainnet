#pragma once

#include <sigmaengine/protocol/authority.hpp>
#include <sigmaengine/protocol/sigmaengine_operations.hpp>

#include <sigmaengine/chain/sigmaengine_object_types.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace sigmaengine { namespace chain {

   using sigmaengine::protocol::digest_type;
   using sigmaengine::protocol::public_key_type;
   using sigmaengine::protocol::version;
   using sigmaengine::protocol::hardfork_version;
   using sigmaengine::protocol::price;
   using sigmaengine::protocol::asset;
   using sigmaengine::protocol::asset_symbol_type;

   /**
    *  All bobservers with at least 1% net positive approval and
    *  at least 2 weeks old are able to participate in block
    *  production.
    */
   class bobserver_object : public object< bobserver_object_type, bobserver_object >
   {
      bobserver_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         bobserver_object( Constructor&& c, allocator< Allocator > a )
            :url( a )
         {
            c( *this );
         }

         id_type           id;

         /** the account that has authority over this bobserver */
         account_name_type account;
         time_point_sec    created;
         shared_string     url;
         uint32_t          total_missed = 0;
         uint64_t          last_aslot = 0;
         uint64_t          last_confirmed_block_num = 0;

         /**
          *  This is the key used to sign blocks on behalf of this bobserver
          */
         public_key_type   signing_key;

         /**
          *  The total votes for this bobserver. This determines how the bobserver is ranked for
          *  scheduling.  The top N bobservers by votes are scheduled every round, every one
          *  else takes turns being scheduled proportional to their votes.
          */
         share_type        votes;

         /**
          * This field represents the SigmaEngine blockchain version the bobserver is running.
          */
         version           running_version;

         hardfork_version  hardfork_version_vote;
         time_point_sec    hardfork_time_vote = SIGMAENGINE_GENESIS_TIME;
         bool              is_bproducer = false;
         bool              is_excepted = false;

         account_name_type bp_owner;
   };

   class bobserver_vote_object : public object< bobserver_vote_object_type, bobserver_vote_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         bobserver_vote_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         bobserver_vote_object(){}

         id_type           id;

         bobserver_id_type   bobserver;
         account_id_type   account;
   };

   class bobserver_schedule_object : public object< bobserver_schedule_object_type, bobserver_schedule_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         bobserver_schedule_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         bobserver_schedule_object(){}

         id_type                                                           id;

         uint32_t                                                          next_shuffle_block_num = 1;
         fc::array< account_name_type, SIGMAENGINE_NUM_BOBSERVERS >        current_shuffled_bobservers;
         uint8_t                                                           num_scheduled_bobservers = 1;
         version                                                           majority_version;

         uint8_t max_voted_bobservers            = SIGMAENGINE_MAX_VOTED_BOBSERVERS_HF0;
         uint8_t max_miner_bobservers            = SIGMAENGINE_MAX_MINER_BOBSERVERS_HF0;
         uint8_t max_runner_bobservers           = SIGMAENGINE_MAX_RUNNER_BOBSERVERS_HF0;
         uint8_t hardfork_required_bobservers    = SIGMAENGINE_HARDFORK_REQUIRED_BOBSERVERS;
   };

   struct by_vote_name;
   struct by_name;
   struct by_is_bp;
   struct by_bp_owner;
   
   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      bobserver_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< bobserver_object, bobserver_id_type, &bobserver_object::id > >,
         ordered_unique< tag< by_name >, member< bobserver_object, account_name_type, &bobserver_object::account > >,
         ordered_unique< tag< by_vote_name >,
            composite_key< bobserver_object,
               member< bobserver_object, share_type, &bobserver_object::votes >,
               member< bobserver_object, account_name_type, &bobserver_object::account >
            >,
            composite_key_compare< std::greater< share_type >, std::less< account_name_type > >
         >,
         ordered_non_unique < tag< by_is_bp >,
            composite_key< bobserver_object,
               member< bobserver_object, bool, &bobserver_object::is_bproducer >,
               member< bobserver_object, account_name_type, &bobserver_object::account >
            >,
            composite_key_compare< std::greater< bool >, std::less< account_name_type > >
         >,
         ordered_unique< tag< by_bp_owner >, 
            composite_key< bobserver_object,
               member< bobserver_object, account_name_type, &bobserver_object::bp_owner >, 
               member< bobserver_object, bobserver_id_type, &bobserver_object::id > 
            >
         >
      >,
      allocator< bobserver_object >
   > bobserver_index;

   struct by_account_bobserver;
   struct by_bobserver_account;
   typedef multi_index_container<
      bobserver_vote_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< bobserver_vote_object, bobserver_vote_id_type, &bobserver_vote_object::id > >,
         ordered_unique< tag<by_account_bobserver>,
            composite_key< bobserver_vote_object,
               member<bobserver_vote_object, account_id_type, &bobserver_vote_object::account >,
               member<bobserver_vote_object, bobserver_id_type, &bobserver_vote_object::bobserver >
            >,
            composite_key_compare< std::less< account_id_type >, std::less< bobserver_id_type > >
         >,
         ordered_unique< tag<by_bobserver_account>,
            composite_key< bobserver_vote_object,
               member<bobserver_vote_object, bobserver_id_type, &bobserver_vote_object::bobserver >,
               member<bobserver_vote_object, account_id_type, &bobserver_vote_object::account >
            >,
            composite_key_compare< std::less< bobserver_id_type >, std::less< account_id_type > >
         >
      >, // indexed_by
      allocator< bobserver_vote_object >
   > bobserver_vote_index;

   typedef multi_index_container<
      bobserver_schedule_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< bobserver_schedule_object, bobserver_schedule_id_type, &bobserver_schedule_object::id > >
      >,
      allocator< bobserver_schedule_object >
   > bobserver_schedule_index;

} }

FC_REFLECT( sigmaengine::chain::bobserver_object,
             (id)
             (account)
             (created)
             (url)(votes)(total_missed)
             (last_aslot)(last_confirmed_block_num)(signing_key)
             (running_version)
             (hardfork_version_vote)(hardfork_time_vote)
             (is_bproducer)
             (is_excepted)
             (bp_owner)
          )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::bobserver_object, sigmaengine::chain::bobserver_index )

FC_REFLECT( sigmaengine::chain::bobserver_vote_object, (id)(bobserver)(account) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::bobserver_vote_object, sigmaengine::chain::bobserver_vote_index )

FC_REFLECT( sigmaengine::chain::bobserver_schedule_object,
             (id)(next_shuffle_block_num)(current_shuffled_bobservers)(num_scheduled_bobservers)
             (majority_version)
             (max_voted_bobservers)
             (max_miner_bobservers)
             (max_runner_bobservers)
             (hardfork_required_bobservers)
          )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::bobserver_schedule_object, sigmaengine::chain::bobserver_schedule_index )