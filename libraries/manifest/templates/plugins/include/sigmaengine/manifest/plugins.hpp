
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sigmaengine { namespace app {

class abstract_plugin;
class application;

} }

namespace sigmaengine { namespace plugin {

void initialize_plugin_factories();
std::shared_ptr< sigmaengine::app::abstract_plugin > create_plugin( const std::string& name, sigmaengine::app::application* app );
std::vector< std::string > get_available_plugins();

} }
