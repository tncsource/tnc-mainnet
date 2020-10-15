#pragma once

#include <sigmaengine/protocol/authority.hpp>
#include <sigmaengine/protocol/sigmaengine_operations.hpp>

#include <sigmaengine/chain/sigmaengine_object_types.hpp>
#include <sigmaengine/chain/account_object.hpp>

#include <boost/multi_index/composite_key.hpp>
#include <boost/multiprecision/cpp_int.hpp>


namespace sigmaengine { namespace chain {

   using sigmaengine::protocol::asset;
   using sigmaengine::protocol::price;
   using sigmaengine::protocol::asset_symbol_type;

   typedef protocol::fixed_string_16 common_fund_name_type;

   class savings_withdraw_object : public object< savings_withdraw_object_type, savings_withdraw_object >
   {
      savings_withdraw_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         savings_withdraw_object( Constructor&& c, allocator< Allocator > a )
            :memo( a )
         {
            c( *this );
         }

         id_type           id;

         account_name_type from;
         account_name_type to;
         shared_string     memo;
         uint32_t          request_id = 0;
         asset             amount;
         asset             total_amount;
         uint8_t           split_pay_order = 0;
         uint8_t           split_pay_month = 0;
         time_point_sec    complete;
   };

   class fund_withdraw_object : public object< fund_withdraw_object_type, fund_withdraw_object >
   {
      fund_withdraw_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         fund_withdraw_object( Constructor&& c, allocator< Allocator > a )
            :fund_name( a ), memo( a )
         {
            c( *this );
         }

         id_type           id;

         account_name_type from;
         shared_string     fund_name;
         shared_string     memo;
         uint32_t          request_id = 0;
         asset             amount;
         time_point_sec    complete;
   };

   class decline_voting_rights_request_object : public object< decline_voting_rights_request_object_type, decline_voting_rights_request_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         decline_voting_rights_request_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         decline_voting_rights_request_object(){}

         id_type           id;

         account_id_type   account;
         time_point_sec    effective_date;
   };

   class account_auth_object : public object< account_auth_object_type, account_auth_object >
   {
      account_auth_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         account_auth_object( Constructor&& c, allocator< Allocator > a )
         :auth_type( a ), auth_token( a )
         {
            c( *this );
         }

         id_type           id;
         account_name_type account;
         shared_string     auth_type;
         shared_string     auth_token;
         time_point_sec    reg_date;
   };

   class common_fund_object : public object< common_fund_object_type, common_fund_object >
   {
      public:
         template< typename Constructor, typename Allocator >
         common_fund_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         common_fund_object() {}

         common_fund_id_type                 id;
         common_fund_name_type               name;
         asset                               fund_balance = asset( 0, SGT_SYMBOL );
         asset                               fund_withdraw_ready = asset( 0, SGT_SYMBOL );
         fc::array<fc::array<double,SIGMAENGINE_MAX_STAKING_MONTH>,SIGMAENGINE_MAX_USER_TYPE>   percent_interest;
         time_point_sec                      last_update;
   };

   class dapp_reward_fund_object : public object< dapp_reward_fund_object_type, dapp_reward_fund_object>
   {
      public:
         template< typename Constructor, typename Allocator >
         dapp_reward_fund_object( Constructor&& c, allocator< Allocator > a )
         {
            c( *this );
         }

         dapp_reward_fund_object(){}

         dapp_reward_fund_id_type         id;
         asset                            fund_balance = asset( 0, SGT_SYMBOL );
         time_point_sec                   last_update;
   };


   struct by_from_rid;
   struct by_to_complete;
   struct by_complete_from_rid;
   typedef multi_index_container<
      savings_withdraw_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id > >,
         ordered_unique< tag< by_from_rid >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, uint32_t, &savings_withdraw_object::request_id >
            >
         >,
         ordered_unique< tag< by_to_complete >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::to >,
               member< savings_withdraw_object, time_point_sec,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, savings_withdraw_id_type, &savings_withdraw_object::id >
            >
         >,
         ordered_unique< tag< by_complete_from_rid >,
            composite_key< savings_withdraw_object,
               member< savings_withdraw_object, time_point_sec,  &savings_withdraw_object::complete >,
               member< savings_withdraw_object, account_name_type,  &savings_withdraw_object::from >,
               member< savings_withdraw_object, uint32_t, &savings_withdraw_object::request_id >
            >
         >
      >,
      allocator< savings_withdraw_object >
   > savings_withdraw_index;
   
   struct by_from_id;
   struct by_complete_from;
   struct by_from_complete;
   struct by_amount_id;
   typedef multi_index_container<
      fund_withdraw_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< fund_withdraw_object, fund_withdraw_id_type, &fund_withdraw_object::id > >,
         ordered_unique< tag< by_from_id >,
            composite_key< fund_withdraw_object,
               member< fund_withdraw_object, account_name_type,  &fund_withdraw_object::from >,
               member< fund_withdraw_object, shared_string,  &fund_withdraw_object::fund_name >,
               member< fund_withdraw_object, uint32_t, &fund_withdraw_object::request_id >
            >,
            composite_key_compare< std::less< account_name_type >, chainbase::strcmp_less, std::less< uint32_t > >
         >,
         ordered_unique< tag< by_complete_from >,
            composite_key< fund_withdraw_object,
               member< fund_withdraw_object, time_point_sec,  &fund_withdraw_object::complete >,
               member< fund_withdraw_object, account_name_type,  &fund_withdraw_object::from >,   
               member< fund_withdraw_object, shared_string,  &fund_withdraw_object::fund_name >,          
               member< fund_withdraw_object, fund_withdraw_id_type, &fund_withdraw_object::id >
            >
         >,
         ordered_unique< tag< by_from_complete >,
            composite_key< fund_withdraw_object,
               member< fund_withdraw_object, account_name_type,  &fund_withdraw_object::from >,
               member< fund_withdraw_object, shared_string,  &fund_withdraw_object::fund_name >,
               member< fund_withdraw_object, time_point_sec,  &fund_withdraw_object::complete >,
               member< fund_withdraw_object, fund_withdraw_id_type, &fund_withdraw_object::id >
            >,
            composite_key_compare< std::less< account_name_type >, chainbase::strcmp_less, std::less< time_point_sec >, std::less< fund_withdraw_id_type > >
         >,
         ordered_unique< tag< by_amount_id >,
            composite_key< fund_withdraw_object,
               member< fund_withdraw_object, shared_string,  &fund_withdraw_object::fund_name >,
               member< fund_withdraw_object, asset,  &fund_withdraw_object::amount >,
               member< fund_withdraw_object, fund_withdraw_id_type, &fund_withdraw_object::id >
            >,
            composite_key_compare< chainbase::strcmp_less, std::greater< asset >, std::less< fund_withdraw_id_type > >
         >
      >,
      allocator< fund_withdraw_object >
   > fund_withdraw_index;

   struct by_name;
   typedef multi_index_container<
      common_fund_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< common_fund_object, common_fund_id_type, &common_fund_object::id > >,
         ordered_unique< tag< by_name >, member< common_fund_object, common_fund_name_type, &common_fund_object::name > >
      >,
      allocator< common_fund_object >
   > common_fund_index;

   struct by_account;
   struct by_effective_date;
   typedef multi_index_container<
      decline_voting_rights_request_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< decline_voting_rights_request_object, decline_voting_rights_request_id_type, &decline_voting_rights_request_object::id > >,
         ordered_unique< tag< by_account >,
            member< decline_voting_rights_request_object, account_id_type, &decline_voting_rights_request_object::account >
         >,
         ordered_unique< tag< by_effective_date >,
            composite_key< decline_voting_rights_request_object,
               member< decline_voting_rights_request_object, time_point_sec, &decline_voting_rights_request_object::effective_date >,
               member< decline_voting_rights_request_object, account_id_type, &decline_voting_rights_request_object::account >
            >,
            composite_key_compare< std::less< time_point_sec >, std::less< account_id_type > >
         >
      >,
      allocator< decline_voting_rights_request_object >
   > decline_voting_rights_request_index;

   struct by_reg_date;
   struct by_auth_type;
   typedef multi_index_container<
      account_auth_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< account_auth_object, account_auth_id_type, &account_auth_object::id > >,
         ordered_unique< tag< by_auth_type >,
            composite_key< account_auth_object,
               member< account_auth_object, account_name_type, &account_auth_object::account >,
               member< account_auth_object, shared_string, &account_auth_object::auth_type >
            >,
            composite_key_compare< std::less< account_name_type >, chainbase::strcmp_less>
         >
      >,
      allocator< account_auth_object >
   > account_auth_index;

   typedef multi_index_container<
      dapp_reward_fund_object,
      indexed_by<
         ordered_unique< tag< by_id >, member< dapp_reward_fund_object, dapp_reward_fund_id_type, &dapp_reward_fund_object::id > >
      >,
      allocator< dapp_reward_fund_object >
   > dapp_reward_fund_index;

} } // sigmaengine::chain


FC_REFLECT( sigmaengine::chain::savings_withdraw_object,
             (id)(from)(to)(memo)(request_id)(amount)(total_amount)(split_pay_order)(split_pay_month)(complete) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::savings_withdraw_object, sigmaengine::chain::savings_withdraw_index )

FC_REFLECT( sigmaengine::chain::common_fund_object,
            (id)
            (name)
            (fund_balance)
            (fund_withdraw_ready)
            (percent_interest)
            (last_update)
         )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::common_fund_object, sigmaengine::chain::common_fund_index )

FC_REFLECT( sigmaengine::chain::fund_withdraw_object,
             (from)(fund_name)(memo)(request_id)(amount)(complete) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::fund_withdraw_object, sigmaengine::chain::fund_withdraw_index )


FC_REFLECT( sigmaengine::chain::decline_voting_rights_request_object,
             (id)(account)(effective_date) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::decline_voting_rights_request_object, sigmaengine::chain::decline_voting_rights_request_index )

FC_REFLECT( sigmaengine::chain::dapp_reward_fund_object,
            (id)
            (fund_balance)
            (last_update)
         )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::dapp_reward_fund_object, sigmaengine::chain::dapp_reward_fund_index )

FC_REFLECT( sigmaengine::chain::account_auth_object,
             (account)(auth_type)(auth_token)(reg_date) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::account_auth_object, sigmaengine::chain::account_auth_index )