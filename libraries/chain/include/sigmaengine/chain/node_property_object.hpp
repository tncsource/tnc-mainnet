#pragma once

namespace sigmaengine { namespace chain {

   /**
    * @brief Contains per-node database configuration.
    *
    *  Transactions are evaluated differently based on per-node state.
    *  Settings here may change based on whether the node is syncing or up-to-date.
    *  Or whether the node is a bobserver node. Or if we're processing a
    *  transaction in a bobserver-signed block vs. a fresh transaction
    *  from the p2p network.  Or configuration-specified tradeoffs of
    *  performance/hardfork resilience vs. paranoia.
    */
   class node_property_object
   {
      public:
         node_property_object(){}
         ~node_property_object(){}

         uint32_t skip_flags = 0;
   };
} } // sigmaengine::chain
