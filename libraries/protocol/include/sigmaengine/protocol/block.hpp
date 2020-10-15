#pragma once
#include <sigmaengine/protocol/block_header.hpp>
#include <sigmaengine/protocol/transaction.hpp>

namespace sigmaengine { namespace protocol {

   struct signed_block : public signed_block_header
   {
      checksum_type calculate_merkle_root()const;
      vector<signed_transaction> transactions;
   };

} } // sigmaengine::protocol

FC_REFLECT_DERIVED( sigmaengine::protocol::signed_block, (sigmaengine::protocol::signed_block_header), (transactions) )
