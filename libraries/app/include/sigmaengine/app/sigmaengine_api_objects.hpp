#pragma once
#include <sigmaengine/chain/account_object.hpp>
#include <sigmaengine/chain/block_summary_object.hpp>
#include <sigmaengine/chain/global_property_object.hpp>
#include <sigmaengine/chain/history_object.hpp>
#include <sigmaengine/chain/sigmaengine_objects.hpp>
#include <sigmaengine/chain/transaction_object.hpp>
#include <sigmaengine/chain/bobserver_objects.hpp>

#include <sigmaengine/bobserver/bobserver_objects.hpp>

namespace sigmaengine { namespace app {

using namespace sigmaengine::chain;

typedef chain::change_recovery_account_request_object  change_recovery_account_request_api_obj;
typedef chain::block_summary_object                    block_summary_api_obj;
typedef chain::decline_voting_rights_request_object    decline_voting_rights_request_api_obj;
typedef chain::bobserver_vote_object                   bobserver_vote_api_obj;
typedef chain::bobserver_schedule_object               bobserver_schedule_api_obj;
typedef chain::dapp_reward_fund_object                 dapp_reward_fund_api_object;
typedef chain::common_fund_object                      common_fund_api_obj;

struct account_api_obj
{
   account_api_obj( const chain::account_object& a, const chain::database& db ) :
      id( a.id ),
      name( a.name ),
      memo_key( a.memo_key ),
      json_metadata( to_string( a.json_metadata ) ),
      last_account_update( a.last_account_update ),
      created( a.created ),
      mined( a.mined ),
      recovery_account( a.recovery_account ),
      reset_account( a.reset_account ),
      last_account_recovery( a.last_account_recovery ),
      balance( a.balance ),
      savings_balance( a.savings_balance ),
      post_count( a.post_count ),
      last_post( a.last_post ),
      last_root_post( a.last_root_post )
   {

      const auto& auth = db.get< account_authority_object, by_account >( name );
      owner = authority( auth.owner );
      active = authority( auth.active );
      posting = authority( auth.posting );
      last_owner_update = auth.last_owner_update;

   }


   account_api_obj(){}

   account_id_type   id;

   account_name_type name;
   authority         owner;
   authority         active;
   authority         posting;
   public_key_type   memo_key;
   string            json_metadata;

   time_point_sec    last_owner_update;
   time_point_sec    last_account_update;

   time_point_sec    created;
   bool              mined = false;
   account_name_type recovery_account;
   account_name_type reset_account;
   time_point_sec    last_account_recovery;

   asset             balance;
   asset             savings_balance;

   uint32_t          post_count = 0;

   time_point_sec    last_post;
   time_point_sec    last_root_post;
};

struct owner_authority_history_api_obj
{
   owner_authority_history_api_obj( const chain::owner_authority_history_object& o ) :
      id( o.id ),
      account( o.account ),
      previous_owner_authority( authority( o.previous_owner_authority ) ),
      last_valid_time( o.last_valid_time )
   {}

   owner_authority_history_api_obj() {}

   owner_authority_history_id_type  id;

   account_name_type                account;
   authority                        previous_owner_authority;
   time_point_sec                   last_valid_time;
};

struct account_recovery_request_api_obj
{
   account_recovery_request_api_obj( const chain::account_recovery_request_object& o ) :
      id( o.id ),
      account_to_recover( o.account_to_recover ),
      new_owner_authority( authority( o.new_owner_authority ) ),
      expires( o.expires )
   {}

   account_recovery_request_api_obj() {}

   account_recovery_request_id_type id;
   account_name_type                account_to_recover;
   authority                        new_owner_authority;
   time_point_sec                   expires;
};

struct account_balance_api_obj {
   account_balance_api_obj(){}

   account_balance_api_obj( account_name_type _name, asset _balance) :
      account( _name ), balance( _balance ){}

   account_name_type    account;
   asset                balance;
};

struct account_history_api_obj
{

};

struct bobserver_api_obj
{
   bobserver_api_obj( const chain::bobserver_object& w ) :
      id( w.id ),
      account( w.account ),
      created( w.created ),
      url( to_string( w.url ) ),
      total_missed( w.total_missed ),
      last_aslot( w.last_aslot ),
      last_confirmed_block_num( w.last_confirmed_block_num ),
      signing_key( w.signing_key ),
      votes( w.votes ),
      running_version( w.running_version ),
      hardfork_version_vote( w.hardfork_version_vote ),
      hardfork_time_vote( w.hardfork_time_vote ),
      is_bproducer( w.is_bproducer ),
      is_excepted( w.is_excepted ),
      bp_owner( w.bp_owner )
   {}

   bobserver_api_obj() {}

   bobserver_id_type   id;
   account_name_type account;
   time_point_sec    created;
   string            url;
   uint32_t          total_missed = 0;
   uint64_t          last_aslot = 0;
   uint64_t          last_confirmed_block_num = 0;
   public_key_type   signing_key;
   share_type        votes;
   version           running_version;
   hardfork_version  hardfork_version_vote;
   time_point_sec    hardfork_time_vote;
   bool              is_bproducer;
   bool              is_excepted;
   account_name_type bp_owner;
};



struct savings_withdraw_api_obj
{
   savings_withdraw_api_obj( const chain::savings_withdraw_object& o ) :
      id( o.id ),
      from( o.from ),
      to( o.to ),
      memo( to_string( o.memo ) ),
      request_id( o.request_id ),
      amount( o.amount ),
      total_amount( o.total_amount ),
      split_pay_order ( o.split_pay_order ),
      split_pay_month ( o.split_pay_month ),
      complete( o.complete )
   {}

   savings_withdraw_api_obj() {}

   savings_withdraw_id_type   id;
   account_name_type          from;
   account_name_type          to;
   string                     memo;
   uint32_t                   request_id = 0;
   asset                      amount;
   asset                      total_amount;
   uint8_t                    split_pay_order = 0;
   uint8_t                    split_pay_month = 0;
   time_point_sec             complete;
};

struct fund_withdraw_api_obj
{
   fund_withdraw_api_obj( const chain::fund_withdraw_object& o ) :
      id( o.id ),
      from( o.from ),
      fund_name( to_string (o.fund_name ) ),
      memo( to_string( o.memo ) ),
      request_id( o.request_id ),
      amount( o.amount ),
      complete( o.complete )
   {}

   fund_withdraw_api_obj() {}

   fund_withdraw_id_type      id;
   account_name_type          from;
   string                     fund_name;
   string                     memo;
   uint32_t                   request_id = 0;
   asset                      amount;
   time_point_sec             complete;
};


struct bproducer_api_obj
{
   bproducer_api_obj( const chain::bobserver_object& w ) :
      account( w.account ),
      votes( w.votes ),
      running_version( w.running_version ),
      hardfork_version_vote( w.hardfork_version_vote ),
      hardfork_time_vote( w.hardfork_time_vote )
   {}

   bproducer_api_obj() {}

   account_name_type account;
   share_type        votes;
   version           running_version;
   hardfork_version  hardfork_version_vote;
   time_point_sec    hardfork_time_vote;
};

struct signed_block_api_obj : public signed_block
{
   signed_block_api_obj( const signed_block& block ) : signed_block( block )
   {
      block_id = id();
      signing_key = signee();
      transaction_ids.reserve( transactions.size() );
      for( const signed_transaction& tx : transactions )
         transaction_ids.push_back( tx.id() );
   }
   signed_block_api_obj() {}

   block_id_type                 block_id;
   public_key_type               signing_key;
   vector< transaction_id_type > transaction_ids;
};

struct dynamic_global_property_api_obj : public dynamic_global_property_object
{
   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo, const chain::database& db ) :
      dynamic_global_property_object( gpo )
   {
      if( db.has_index< bobserver::reserve_ratio_index >() )
      {
         const auto& r = db.find( bobserver::reserve_ratio_id_type() );

         if( BOOST_LIKELY( r != nullptr ) )
         {
            current_reserve_ratio = r->current_reserve_ratio;
            average_block_size = r->average_block_size;
         }
      }
   }

   dynamic_global_property_api_obj( const dynamic_global_property_object& gpo ) :
      dynamic_global_property_object( gpo ) {}

   dynamic_global_property_api_obj() {}

   uint32_t    current_reserve_ratio = 0;
   uint64_t    average_block_size = 0;
};

struct account_auth_api_obj
{
   account_auth_api_obj( const chain::account_auth_object& o ) :
      id( o.id ),
      account( o.account ),
      auth_type( to_string(o.auth_type) ),
      auth_token( to_string(o.auth_token) ),
      reg_date( o.reg_date )
   {}

   account_auth_api_obj() {}

   account_auth_id_type  id;
   account_name_type     account;
   string                auth_type;
   string                auth_token;
   time_point_sec        reg_date;
};

} } // sigmaengine::app

FC_REFLECT( sigmaengine::app::account_api_obj,
             (id)(name)(owner)(active)(posting)(memo_key)(json_metadata)(last_owner_update)(last_account_update)
             (created)(mined)
             (recovery_account)(last_account_recovery)(reset_account)
             (balance)
             (savings_balance)
             (post_count)
             (last_post)(last_root_post)
          )

FC_REFLECT( sigmaengine::app::owner_authority_history_api_obj,
             (id)
             (account)
             (previous_owner_authority)
             (last_valid_time)
          )

FC_REFLECT( sigmaengine::app::account_recovery_request_api_obj,
             (id)
             (account_to_recover)
             (new_owner_authority)
             (expires)
          )

FC_REFLECT( sigmaengine::app::account_balance_api_obj,
            (account)
            (balance)
         )

FC_REFLECT( sigmaengine::app::bobserver_api_obj,
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

FC_REFLECT( sigmaengine::app::bproducer_api_obj,
            (account)
            (votes)
            (running_version)
            (hardfork_version_vote)
            (hardfork_time_vote)
         )

FC_REFLECT_DERIVED( sigmaengine::app::signed_block_api_obj, (sigmaengine::protocol::signed_block),
             (block_id)
             (signing_key)
             (transaction_ids)
          )

FC_REFLECT_DERIVED( sigmaengine::app::dynamic_global_property_api_obj, (sigmaengine::chain::dynamic_global_property_object),
             (current_reserve_ratio)
             (average_block_size)
          )

FC_REFLECT( sigmaengine::app::account_auth_api_obj,
            (id)
            (account)
            (auth_type)
            (auth_token)
            (reg_date)
         )

FC_REFLECT( sigmaengine::app::savings_withdraw_api_obj,
             (id)
             (from)
             (to)
             (memo)
             (request_id)
             (amount)
             (total_amount)
             (split_pay_order)
             (split_pay_month)
             (complete)
          )

FC_REFLECT( sigmaengine::app::fund_withdraw_api_obj,
             (id)
             (from)
             (fund_name)
             (memo)
             (request_id)
             (amount)
             (complete)
          )
