#include <sigmaengine/bobserver/bobserver_operations.hpp>

#include <sigmaengine/protocol/operation_util_impl.hpp>

namespace sigmaengine { namespace bobserver {

void enable_content_editing_operation::validate()const
{
   chain::validate_account_name( account );
}

} } // sigmaengine::bobserver

DEFINE_OPERATION_TYPE( sigmaengine::bobserver::bobserver_plugin_operation )
