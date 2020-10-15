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
#pragma once

#include <fc/container/flat.hpp>
#include <sigmaengine/protocol/operations.hpp>
#include <sigmaengine/protocol/types.hpp>
#include <sigmaengine/chain/sigmaengine_object_types.hpp>
#include <sigmaengine/chain/database.hpp>

#include <sigmaengine/token/token_operations.hpp>
#include <sigmaengine/dapp/dapp_operations.hpp>
#include <sigmaengine/bobserver/bobserver_operations.hpp>


#include <fc/string.hpp>

namespace sigmaengine { namespace dapp_history {
   using namespace fc;
   using namespace sigmaengine::token;
   using namespace sigmaengine::dapp;
   using namespace sigmaengine::bobserver;

   template< typename OPERATION_TYPE, typename VISITOR >
   void process_inner_operation( const fc::variant var, VISITOR visitor );

   template< typename OPERATION_TYPE, typename VISITOR >
   void process_inner_operation( vector< char > data, VISITOR visitor );

   void get_imapcted_dapp_from_custom( const custom_json_dapp_operation& op, chain::database& db, fc::flat_set< protocol::dapp_name_type >& result );
   void get_imapcted_dapp_from_custom( const custom_json_operation& op, chain::database& db, fc::flat_set< protocol::dapp_name_type >& result );
   void get_imapcted_dapp_from_custom( const custom_binary_operation& op, chain::database& db, fc::flat_set< protocol::dapp_name_type >& result );

   void operation_get_impacted_dapp(
      const sigmaengine::protocol::operation& op,
      chain::database& db,
      fc::flat_set< protocol::dapp_name_type >& result );

} } // sigmaengine::dapp_history
