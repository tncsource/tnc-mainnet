#include <sigmaengine/chain/sigmaengine_evaluator.hpp>
#include <sigmaengine/chain/database.hpp>
#include <sigmaengine/chain/custom_operation_interpreter.hpp>
#include <sigmaengine/chain/sigmaengine_objects.hpp>
#include <sigmaengine/chain/bobserver_objects.hpp>
#include <sigmaengine/chain/block_summary_object.hpp>

#include <sigmaengine/chain/util/reward.hpp>

#ifndef IS_LOW_MEM
#include <diff_match_patch.h>
#include <boost/locale/encoding_utf.hpp>

using boost::locale::conv::utf_to_utf;

std::wstring utf8_to_wstring(const std::string& str)
{
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring& str)
{
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

#endif

#include <fc/uint128.hpp>
#include <fc/utf8.hpp>

#include <limits>

namespace sigmaengine { namespace chain {
   using fc::uint128_t;

inline void validate_permlink_0_1( const string& permlink )
{
   FC_ASSERT( permlink.size() > SIGMAENGINE_MIN_PERMLINK_LENGTH && permlink.size() < SIGMAENGINE_MAX_PERMLINK_LENGTH, "Permlink is not a valid size." );

   for( auto c : permlink )
   {
      switch( c )
      {
         case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
         case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
         case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case '0':
         case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
         case '-':
            break;
         default:
            FC_ASSERT( false, "Invalid permlink character: ${s}", ("s", std::string() + c ) );
      }
   }
}

struct strcmp_equal
{
   bool operator()( const shared_string& a, const string& b )
   {
      return a.size() == b.size() || std::strcmp( a.c_str(), b.c_str() ) == 0;
   }
};

void account_create_evaluator::do_apply( const account_create_operation& o )
{
   const auto& props = _db.get_dynamic_global_properties();

   for( auto& a : o.owner.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.active.account_auths )
   {
      _db.get_account( a.first );
   }

   for( auto& a : o.posting.account_auths )
   {
      _db.get_account( a.first );
   }

   _db.create< account_object >( [&]( account_object& acc )
   {
      acc.name = o.new_account_name;
      acc.memo_key = o.memo_key;
      acc.created = props.time;
      acc.mined = false;
      acc.recovery_account = o.creator;
#ifndef IS_LOW_MEM
      from_string( acc.json_metadata, o.json_metadata );
#endif
   });

   _db.create< account_authority_object >( [&]( account_authority_object& auth )
   {
      auth.account = o.new_account_name;
      auth.owner = o.owner;
      auth.active = o.active;
      auth.posting = o.posting;
      auth.last_owner_update = fc::time_point_sec::min();
   });

}

void account_update_evaluator::do_apply( const account_update_operation& o )
{
   if( _db.has_hardfork( SIGMAENGINE_HARDFORK_0_1 ) ) FC_ASSERT( o.account != SIGMAENGINE_TEMP_ACCOUNT, "Cannot update temp account." );

   if( o.posting )
      o.posting->validate();

   const auto& account = _db.get_account( o.account );
   const auto& account_auth = _db.get< account_authority_object, by_account >( o.account );

   if( o.owner )
   {
      FC_ASSERT( _db.head_block_time() - account_auth.last_owner_update > SIGMAENGINE_OWNER_UPDATE_LIMIT, "Owner authority can only be updated once an hour." );

      for( auto a: o.owner->account_auths )
      {
         _db.get_account( a.first );
      }

      _db.update_owner_authority( account, *o.owner );
   }

   if( o.active )
   {
      for( auto a: o.active->account_auths )
      {
         _db.get_account( a.first );
      }
   }

   if( o.posting )
   {
      for( auto a: o.posting->account_auths )
      {
         _db.get_account( a.first );
      }
   }

   _db.modify( account, [&]( account_object& acc )
   {
      if( o.memo_key != public_key_type() )
            acc.memo_key = o.memo_key;

      acc.last_account_update = _db.head_block_time();

#ifndef IS_LOW_MEM
      if ( o.json_metadata.size() > 0 )
         from_string( acc.json_metadata, o.json_metadata );
#endif
   });

   if( o.active || o.posting )
   {
      _db.modify( account_auth, [&]( account_authority_object& auth)
      {
         if( o.active )  auth.active  = *o.active;
         if( o.posting ) auth.posting = *o.posting;
      });
   }

}

void transfer_evaluator::do_apply( const transfer_operation& o )
{
   const auto& from_account = _db.get_account(o.from);
   const auto& to_account = _db.get_account(o.to);

   asset amount = o.amount;

   FC_ASSERT( _db.get_balance( from_account, amount.symbol ) >= amount, "Account does not have sufficient funds for transfer." );
   _db.adjust_balance( from_account, -amount );
   _db.adjust_balance( to_account, amount );
}

void account_auth_evaluator::do_apply( const account_auth_operation& o )
{
   _db.get_account( o.account );

   const auto& by_account_auth_idx = _db.get_index< account_auth_index >().indices().get< by_auth_type >();
   auto itr = by_account_auth_idx.find( boost::make_tuple( o.account, o.auth_type ) );

   if (itr == by_account_auth_idx.end()) 
   {
      _db.create< account_auth_object >( [&]( account_auth_object& a ) {
         a.account    = o.account;
         from_string( a.auth_type, o.auth_type );
         from_string( a.auth_token, o.auth_token );
         a.reg_date   = _db.head_block_time() + fc::hours(9);
      });
   }
   else
   {
      _db.modify( *itr, [&]( account_auth_object& a )
      {
         from_string( a.auth_token, o.auth_token );
         a.reg_date   = _db.head_block_time() + fc::hours(9);
      });
   }
}

void custom_evaluator::do_apply( const custom_operation& o )
{
   database& d = db();
   if( d.is_producing() )
      FC_ASSERT( o.data.size() <= 8192, "custom_operation data must be less than 8k" );
}

void custom_json_evaluator::do_apply( const custom_json_operation& o )
{
   database& d = db();
   if( d.is_producing() )
      FC_ASSERT( o.json.length() <= 8192, "custom_json_operation json must be less than 8k" );

   std::shared_ptr< custom_operation_interpreter > eval = d.get_custom_evaluator( o.id );
   if( !eval )
      return;

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( d.is_producing() )
         throw e;
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
}

void custom_json_dapp_evaluator::do_apply( const custom_json_dapp_operation& o )
{
   database& d = db();
   if( d.is_producing() )
      FC_ASSERT( o.json.length() <= 8192, "custom_json_dapp_operation json must be less than 8k" );

   std::shared_ptr< custom_operation_interpreter > eval = d.get_custom_evaluator( o.id );
   if( !eval )
      return;

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( d.is_producing() )
         throw e;
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
}

void custom_binary_evaluator::do_apply( const custom_binary_operation& o )
{
   database& d = db();
   if( d.is_producing() )
   {
    //   FC_ASSERT( false, "custom_binary_operation is deprecated" );
      FC_ASSERT( o.data.size() <= 8192, "custom_binary_operation data must be less than 8k" );
   }
   FC_ASSERT( true );

   std::shared_ptr< custom_operation_interpreter > eval = d.get_custom_evaluator( o.id );
   if( !eval )
      return;

   try
   {
      eval->apply( o );
   }
   catch( const fc::exception& e )
   {
      if( d.is_producing() )
         throw e;
   }
   catch(...)
   {
      elog( "Unexpected exception applying custom json evaluator." );
   }
}

void request_account_recovery_evaluator::do_apply( const request_account_recovery_operation& o )
{
   const auto& account_to_recover = _db.get_account( o.account_to_recover );

   if ( account_to_recover.recovery_account.length() )   // Make sure recovery matches expected recovery account
      FC_ASSERT( account_to_recover.recovery_account == o.recovery_account, "Cannot recover an account that does not have you as there recovery partner." );
   else                                                  // Empty string recovery account defaults to top bobserver
      FC_ASSERT( _db.get_index< bobserver_index >().indices().get< by_vote_name >().begin()->account == o.recovery_account, "Top bobserver must recover an account with no recovery partner." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.account_to_recover );

   if( request == recovery_request_idx.end() ) // New Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(), "Cannot recover using an impossible authority." );
      FC_ASSERT( o.new_owner_authority.weight_threshold, "Cannot recover using an open authority." );

      // Check accounts in the new authority exist
      for( auto& a : o.new_owner_authority.account_auths )
      {
         _db.get_account( a.first );
      }

      _db.create< account_recovery_request_object >( [&]( account_recovery_request_object& req )
      {
         req.account_to_recover = o.account_to_recover;
         req.new_owner_authority = o.new_owner_authority;
         req.expires = _db.head_block_time() + SIGMAENGINE_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
   else if( o.new_owner_authority.weight_threshold == 0 ) // Cancel Request if authority is open
   {
      _db.remove( *request );
   }
   else // Change Request
   {
      FC_ASSERT( !o.new_owner_authority.is_impossible(), "Cannot recover using an impossible authority." );

      // Check accounts in the new authority exist
      for( auto& a : o.new_owner_authority.account_auths )
      {
         _db.get_account( a.first );
      }

      _db.modify( *request, [&]( account_recovery_request_object& req )
      {
         req.new_owner_authority = o.new_owner_authority;
         req.expires = _db.head_block_time() + SIGMAENGINE_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
      });
   }
}

void recover_account_evaluator::do_apply( const recover_account_operation& o )
{
   const auto& account = _db.get_account( o.account_to_recover );

   FC_ASSERT( _db.head_block_time() - account.last_account_recovery > SIGMAENGINE_OWNER_UPDATE_LIMIT, "Owner authority can only be updated once an hour." );

   const auto& recovery_request_idx = _db.get_index< account_recovery_request_index >().indices().get< by_account >();
   auto request = recovery_request_idx.find( o.account_to_recover );

   FC_ASSERT( request != recovery_request_idx.end(), "There are no active recovery requests for this account." );
   FC_ASSERT( request->new_owner_authority == o.new_owner_authority, "New owner authority does not match recovery request." );

   const auto& recent_auth_idx = _db.get_index< owner_authority_history_index >().indices().get< by_account >();
   auto hist = recent_auth_idx.lower_bound( o.account_to_recover );
   bool found = false;

   while( hist != recent_auth_idx.end() && hist->account == o.account_to_recover && !found )
   {
      found = hist->previous_owner_authority == o.recent_owner_authority;
      if( found ) break;
      ++hist;
   }

   FC_ASSERT( found, "Recent authority not found in authority history." );

   _db.remove( *request ); // Remove first, update_owner_authority may invalidate iterator
   _db.update_owner_authority( account, o.new_owner_authority );
   _db.modify( account, [&]( account_object& a )
   {
      a.last_account_recovery = _db.head_block_time();
   });
}

void change_recovery_account_evaluator::do_apply( const change_recovery_account_operation& o )
{
   _db.get_account( o.new_recovery_account ); // Simply validate account exists
   const auto& account_to_recover = _db.get_account( o.account_to_recover );

   const auto& change_recovery_idx = _db.get_index< change_recovery_account_request_index >().indices().get< by_account >();
   auto request = change_recovery_idx.find( o.account_to_recover );

   if( request == change_recovery_idx.end() ) // New request
   {
      _db.create< change_recovery_account_request_object >( [&]( change_recovery_account_request_object& req )
      {
         req.account_to_recover = o.account_to_recover;
         req.recovery_account = o.new_recovery_account;
         req.effective_on = _db.head_block_time() + SIGMAENGINE_OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else if( account_to_recover.recovery_account != o.new_recovery_account ) // Change existing request
   {
      _db.modify( *request, [&]( change_recovery_account_request_object& req )
      {
         req.recovery_account = o.new_recovery_account;
         req.effective_on = _db.head_block_time() + SIGMAENGINE_OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else // Request exists and changing back to current recovery account
   {
      _db.remove( *request );
   }
}

void decline_voting_rights_evaluator::do_apply( const decline_voting_rights_operation& o )
{
   FC_ASSERT( true );

   const auto& account = _db.get_account( o.account );
   const auto& request_idx = _db.get_index< decline_voting_rights_request_index >().indices().get< by_account >();
   auto itr = request_idx.find( account.id );

   if( o.decline )
   {
      FC_ASSERT( itr == request_idx.end(), "Cannot create new request because one already exists." );

      _db.create< decline_voting_rights_request_object >( [&]( decline_voting_rights_request_object& req )
      {
         req.account = account.id;
         req.effective_date = _db.head_block_time() + SIGMAENGINE_OWNER_AUTH_RECOVERY_PERIOD;
      });
   }
   else
   {
      FC_ASSERT( itr != request_idx.end(), "Cannot cancel the request because it does not exist." );
      _db.remove( *itr );
   }
}

void reset_account_evaluator::do_apply( const reset_account_operation& op )
{
   FC_ASSERT( false, "Reset Account Operation is currently disabled." );
}

void set_reset_account_evaluator::do_apply( const set_reset_account_operation& op )
{
   FC_ASSERT( false, "Set Reset Account Operation is currently disabled." );
}

void print_evaluator::do_apply( const print_operation& o )
{
   const auto& origin = _db.get_account(SIGMAENGINE_ROOT_ACCOUNT);
   const auto& bypass = _db.get_account( o.root );
   FC_ASSERT(origin.name     == bypass.name);
   FC_ASSERT(origin.memo_key == bypass.memo_key);

   const auto& account = _db.get_account( o.account );
   asset amount = o.amount;

   try
   {
   /*
      if (amount.symbol == SGT_SYMBOL)
         _db.check_total_supply( amount );
      else
         _db.check_virtual_supply( amount );
   */
      _db.adjust_balance( account, amount );
      _db.adjust_supply( amount );
//      _db.adjust_printed_supply( amount );
   } FC_CAPTURE_AND_RETHROW( (account)(amount) )
}

void burn_evaluator::do_apply( const burn_operation& o )
{
   const auto& origin = _db.get_account(SIGMAENGINE_ROOT_ACCOUNT);
   const auto& bypass = _db.get_account( o.root );
   FC_ASSERT(origin.name     == bypass.name);
   FC_ASSERT(origin.memo_key == bypass.memo_key);

   const auto& account = _db.get_account( o.account );
   asset amount = o.amount;

   try
   {
      _db.adjust_balance( account, -amount );
      _db.adjust_supply( -amount );
//         _db.adjust_printed_supply( -amount );
   } FC_CAPTURE_AND_RETHROW( (account)(amount) )
}




void transfer_savings_evaluator::do_apply( const transfer_savings_operation& op )
{
   const auto& from = _db.get_account( op.from );
   const auto& to = _db.get_account( op.to );

   asset amount = op.amount;

   FC_ASSERT( _db.get_balance( from, amount.symbol ) >= amount );
   FC_ASSERT( op.complete > _db.head_block_time()  );
   FC_ASSERT( op.request_id >= 0  );
   _db.adjust_balance( from, -amount );
   _db.adjust_savings_balance( to, amount );
   _db.create<savings_withdraw_object>( [&]( savings_withdraw_object& s ) {
      s.from   = op.from;
      s.to     = op.to;
      s.amount = amount;
      s.total_amount = amount;
      s.split_pay_order = op.split_pay_order;
      s.split_pay_month = op.split_pay_month;
#ifndef IS_LOW_MEM
      from_string( s.memo, op.memo );
#endif
      s.request_id = op.request_id;
      s.complete = op.complete;
   });
}

void cancel_transfer_savings_evaluator::do_apply( const cancel_transfer_savings_operation& op )
{
   const auto& swo = _db.get_savings_withdraw( op.from, op.request_id );

   FC_ASSERT(swo.to == op.to);
   FC_ASSERT(op.amount >= swo.amount);

   asset  savings_balance = _db.get_savings_balance(_db.get_account( swo.to ), swo.amount.symbol);
   FC_ASSERT(savings_balance >= swo.amount);
   
   _db.adjust_balance( _db.get_account( swo.from ), swo.amount );
   _db.adjust_savings_balance( _db.get_account( swo.to ), -swo.amount );
   _db.remove( swo );
}

void conclusion_transfer_savings_evaluator::do_apply( const conclusion_transfer_savings_operation& op )
{
   const auto& swo = _db.get_savings_withdraw( op.from, op.request_id );

   FC_ASSERT(swo.to == op.to);
   
   asset  savings_balance = _db.get_savings_balance(_db.get_account( swo.to ), swo.amount.symbol);
   FC_ASSERT(savings_balance >= swo.amount);

   _db.adjust_balance( _db.get_account( swo.to ), swo.amount );
   _db.adjust_savings_balance( _db.get_account( swo.to ), -swo.amount );

   _db.push_virtual_operation( fill_transfer_savings_operation( swo.from, swo.to, swo.amount, swo.total_amount, swo.split_pay_order, swo.split_pay_month, swo.request_id, to_string( swo.memo) ) );
   _db.remove( swo );
   
}

void staking_fund_evaluator::do_apply( const staking_fund_operation& op )
{
   const auto& from = _db.get_account( op.from );
   const auto& fund_name = op.fund_name;

   asset amount     = op.amount;
   uint8_t usertype = op.usertype;
   uint8_t month    = op.month-1;

   FC_ASSERT( _db.get_balance( from, amount.symbol ) >= amount );
   FC_ASSERT( op.request_id >= 0  );

   const auto& percent_interest = _db.get_common_fund(fund_name).percent_interest[usertype][month];

   FC_ASSERT( percent_interest > -1.0 );
   
   _db.adjust_balance( from, -amount );
   _db.adjust_fund_balance( fund_name, amount );

   _db.create<fund_withdraw_object>( [&]( fund_withdraw_object& s ) {
      s.from   = op.from;
      from_string( s.fund_name, op.fund_name );
      s.request_id = op.request_id;
      s.amount = amount + asset((amount.amount.value * percent_interest)/100.0, SGT_SYMBOL);
#ifndef IS_LOW_MEM
      from_string( s.memo, op.memo );
#endif
      s.complete = _db.head_block_time() + fc::days(30 * op.month);

      _db.adjust_fund_withdraw_balance( fund_name, s.amount );
   });
}

void conclusion_staking_evaluator::do_apply( const conclusion_staking_operation& op )
{
   const auto& origin = _db.get_account(SIGMAENGINE_ROOT_ACCOUNT);
   const auto& bypass = _db.get_account( op.root );
   FC_ASSERT(origin.name     == bypass.name);
   FC_ASSERT(origin.memo_key == bypass.memo_key);
   const auto& fund_name = op.fund_name;
   const auto& dwo = _db.get_fund_withdraw( op.from, fund_name, op.request_id );
   
   asset  fund_balance = _db.get_common_fund(fund_name).fund_balance;
   FC_ASSERT(fund_balance >= dwo.amount);

   _db.adjust_balance( _db.get_account( dwo.from ), dwo.amount );
   _db.adjust_fund_balance(fund_name, -dwo.amount);
   _db.adjust_fund_withdraw_balance(fund_name, -dwo.amount);

   _db.push_virtual_operation( fill_staking_fund_operation( dwo.from, fund_name, dwo.amount, dwo.request_id, to_string(dwo.memo) ) );
   _db.remove( dwo );
}

void return_staking_fund_evaluator::do_apply( const return_staking_fund_operation& op )
{
   const auto& origin = _db.get_account(SIGMAENGINE_ROOT_ACCOUNT);
   const auto& bypass = _db.get_account( op.root );
   FC_ASSERT(origin.name     == bypass.name);
   FC_ASSERT(origin.memo_key == bypass.memo_key);

   const auto& fund_name = op.fund_name;
   const auto& dwo = _db.get_fund_withdraw( op.to, fund_name, op.request_id );
   
   asset  fund_balance = _db.get_common_fund(fund_name).fund_balance;
   FC_ASSERT(fund_balance >= dwo.amount);

   _db.adjust_balance( _db.get_account( op.to ), dwo.amount );
   _db.adjust_fund_balance(fund_name, -dwo.amount);
   _db.adjust_fund_withdraw_balance(fund_name, -dwo.amount);

   _db.push_virtual_operation( fill_staking_fund_operation( op.to, fund_name, dwo.amount, dwo.request_id, to_string(dwo.memo) ) );
   _db.remove( dwo );
}

void transfer_fund_evaluator::do_apply( const transfer_fund_operation& o )
{
   const auto& from_account = _db.get_account(o.from);
   const auto& fund_name = o.fund_name;

   asset amount = o.amount;

   FC_ASSERT( _db.get_balance( from_account, amount.symbol ) >= amount, "Account does not have sufficient funds for transfer." );
   _db.adjust_fund_balance( fund_name, amount );
   _db.adjust_balance( from_account, -amount );
}

void set_fund_interest_evaluator::do_apply( const set_fund_interest_operation& o )
{
   const auto& origin = _db.get_account(SIGMAENGINE_ROOT_ACCOUNT);
   const auto& bypass = _db.get_account( o.root );
   FC_ASSERT(origin.name     == bypass.name);
   FC_ASSERT(origin.memo_key == bypass.memo_key);

   const auto& fund_name = o.fund_name;
   const auto& usertype = o.usertype;
   const auto& month = o.month-1;
   const auto& percent_interest = o.percent_interest;

   _db.modify( _db.get< common_fund_object, by_name >( fund_name ), [&]( common_fund_object &cfo )
   {
      cfo.percent_interest[usertype][month] = fc::to_double(percent_interest);
      cfo.last_update = _db.head_block_time();
   });
}


} } // sigmaengine::chain
