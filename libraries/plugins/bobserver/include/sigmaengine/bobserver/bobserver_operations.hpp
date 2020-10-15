#pragma once

#include <sigmaengine/protocol/base.hpp>
#include <sigmaengine/protocol/operation_util.hpp>

#include <sigmaengine/app/plugin.hpp>

namespace sigmaengine { namespace bobserver {

using namespace std;
using sigmaengine::protocol::base_operation;
using sigmaengine::chain::database;

class bobserver_plugin;

struct enable_content_editing_operation : base_operation
{
   protocol::account_name_type   account;
   fc::time_point_sec            relock_time;

   void validate()const;

   void get_required_active_authorities( flat_set< protocol::account_name_type>& a )const { a.insert( account ); }
};

typedef fc::static_variant<
         enable_content_editing_operation
      > bobserver_plugin_operation;

DEFINE_PLUGIN_EVALUATOR( bobserver_plugin, bobserver_plugin_operation, enable_content_editing );

} } // sigmaengine::bobserver

FC_REFLECT( sigmaengine::bobserver::enable_content_editing_operation, (account)(relock_time) )

FC_REFLECT_TYPENAME( sigmaengine::bobserver::bobserver_plugin_operation )

DECLARE_OPERATION_TYPE( sigmaengine::bobserver::bobserver_plugin_operation )
