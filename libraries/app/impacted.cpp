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

#include <sigmaengine/protocol/authority.hpp>

#include <sigmaengine/chain/database.hpp>
#include <sigmaengine/chain/custom_operation_interpreter.hpp>

#include <sigmaengine/app/impacted.hpp>

#include <fc/utility.hpp>

namespace sigmaengine { namespace app {

using namespace fc;
using namespace sigmaengine::protocol;

// TODO:  Review all of these, especially no-ops
struct get_impacted_account_visitor
{
   flat_set<account_name_type>& _impacted;
   chain::database& _db;
   get_impacted_account_visitor( chain::database& db, flat_set<account_name_type>& impact ): _impacted( impact ), _db( db ) {}
   typedef void result_type;

   template<typename T>
   void operator()( const T& op )
   {
      op.get_required_posting_authorities( _impacted );
      op.get_required_active_authorities( _impacted );
      op.get_required_owner_authorities( _impacted );
   }

   // ops
   void operator()( const account_create_operation& op )
   {
      _impacted.insert( op.new_account_name );
      _impacted.insert( op.creator );
   }

   void operator()( const bobserver_update_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const transfer_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const request_account_recovery_operation& op )
   {
      _impacted.insert( op.account_to_recover );
      _impacted.insert( op.recovery_account );
   }

   void operator()( const recover_account_operation& op )
   {
      _impacted.insert( op.account_to_recover );
   }

   void operator()( const change_recovery_account_operation& op )
   {
      _impacted.insert( op.account_to_recover );
   }

   void operator()( const shutdown_bobserver_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const update_bproducer_operation& op )
   {
      _impacted.insert( op.bobserver );
   }

   void operator()( const except_bobserver_operation& op )
   {
      _impacted.insert( op.bobserver );
   }

   void operator()( const account_auth_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const custom_json_operation& op) {
      dlog("IMPACT : custom_json_operation ");
      get_impacted_account_from_custom( op, _impacted );
   }

   void operator()( const custom_json_dapp_operation& op) {
      dlog("IMPACT : custom_json_dapp_operation ");
      get_impacted_account_from_custom( op, _impacted );
   }

   void operator()( const custom_binary_operation& op) {
      dlog("IMPACT : custom_binary_operation ");
      get_impacted_account_from_custom( op, _impacted );
   }

   void operator()( const print_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.root );
   }

   void operator()( const burn_operation& op )
   {
      _impacted.insert( op.account );
      _impacted.insert( op.root );
   }

   
   void operator()( const transfer_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const fill_transfer_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const cancel_transfer_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const conclusion_transfer_savings_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }

   void operator()( const staking_fund_operation& op )
   {
      _impacted.insert( op.from );
   }

   void operator()( const fill_staking_fund_operation& op )
   {
      _impacted.insert( op.from );
   }

   void operator()( const conclusion_staking_operation& op )
   {
      _impacted.insert( op.from );
   }

   void operator()( const return_staking_fund_operation& op )
   {
      _impacted.insert( op.root );
   }

   void operator()( const transfer_fund_operation& op )
   {
      _impacted.insert( op.from );
   }

   void operator()( const set_fund_interest_operation& op )
   {
      _impacted.insert( op.root );
   }
   
};

struct get_account_visitor_from_custom
{
   flat_set<account_name_type>& _impacted;
   get_account_visitor_from_custom( flat_set<account_name_type>& impact ):_impacted( impact ) {}
   typedef void result_type;

   template<typename T>
   void operator()( const T& op )
   {
      op.get_required_posting_authorities( _impacted );
      op.get_required_active_authorities( _impacted );
      op.get_required_owner_authorities( _impacted );
   }
};

template< typename OPERATION_TYPE, typename VISITOR >
void process_inner_operation( const fc::variant var, VISITOR visitor ){
   try {
      std::vector< OPERATION_TYPE > operations;
      if( var.is_array() && var.size() > 0 && var.get_array()[0].is_array() ) {
         from_variant( var, operations );
      } else {
         operations.emplace_back();
         from_variant( var, operations[0] );
      }

      for( const OPERATION_TYPE& inner_o : operations ) {
         inner_o.visit( visitor );
      }
   } catch( const fc::exception& ) { }
}

template< typename OPERATION_TYPE, typename VISITOR >
void process_inner_operation( vector< char > data, VISITOR visitor ){
   try {
      std::vector< OPERATION_TYPE > operations;
      try {
         operations = fc::raw::unpack< vector< OPERATION_TYPE > >( data );
      } catch ( fc::exception& ) {
         operations.push_back( fc::raw::unpack< OPERATION_TYPE >( data ) );
      }

      for( const OPERATION_TYPE& inner_o : operations ) {
         inner_o.visit( visitor );
      }
   } catch( const fc::exception& ) { }
}

void get_impacted_account_from_custom( const custom_json_dapp_operation& op, flat_set< account_name_type >& result ) {
   auto var = fc::json::from_string( op.json );

   process_inner_operation< dapp_operation >( var, get_account_visitor_from_custom( result ) );
   process_inner_operation< token_operation >( var, get_account_visitor_from_custom( result ) );
   process_inner_operation< bobserver_plugin_operation >( var, get_account_visitor_from_custom( result ) );
}

void get_impacted_account_from_custom( const custom_json_operation& op, flat_set< account_name_type >& result ) {
   auto var = fc::json::from_string( op.json );

   process_inner_operation< dapp_operation >( var, get_account_visitor_from_custom( result ) );
   process_inner_operation< token_operation >( var, get_account_visitor_from_custom( result ) );
   process_inner_operation< bobserver_plugin_operation, get_account_visitor_from_custom >( var, get_account_visitor_from_custom( result ) );
}

void get_impacted_account_from_custom( const custom_binary_operation& op, flat_set< account_name_type >& result ) {

   process_inner_operation< dapp_operation >( op.data, get_account_visitor_from_custom( result ) );
   process_inner_operation< token_operation >( op.data, get_account_visitor_from_custom( result ) );
   process_inner_operation< bobserver_plugin_operation, get_account_visitor_from_custom >( op.data, get_account_visitor_from_custom( result ) );
}

// template<>
void operation_get_impacted_accounts( const operation& op, chain::database& db, flat_set<account_name_type>& result )
{
   get_impacted_account_visitor vtor = get_impacted_account_visitor( db, result );
   op.visit( vtor );
}

void transaction_get_impacted_accounts( const transaction& tx, chain::database& db, flat_set<account_name_type>& result )
{
   for( const auto& op : tx.operations )
      operation_get_impacted_accounts( op, db, result );
}

} }
