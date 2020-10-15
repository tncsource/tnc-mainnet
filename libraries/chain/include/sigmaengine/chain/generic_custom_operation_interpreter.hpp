
#pragma once

#include <sigmaengine/protocol/sigmaengine_operations.hpp>
#include <sigmaengine/protocol/operation_util_impl.hpp>

#include <sigmaengine/chain/evaluator.hpp>
#include <sigmaengine/chain/evaluator_registry.hpp>
#include <sigmaengine/chain/custom_operation_interpreter.hpp>

#include <graphene/schema/schema.hpp>

#include <fc/variant.hpp>

#include <string>
#include <vector>

namespace sigmaengine { namespace chain {

class database;

template< typename CustomOperationType >
class generic_custom_operation_interpreter
   : public custom_operation_interpreter, public evaluator_registry< CustomOperationType >
{
   public:
      generic_custom_operation_interpreter( database& db ) : evaluator_registry< CustomOperationType >(db) {}

      void apply_operations( const vector< CustomOperationType >& custom_operations, const operation& outer_o )
      {
         auto plugin_session = this->_db.start_undo_session( true );

         flat_set<account_name_type> outer_active;
         flat_set<account_name_type> outer_owner;
         flat_set<account_name_type> outer_posting;
         std::vector< authority >    outer_other;

         flat_set<account_name_type> inner_active;
         flat_set<account_name_type> inner_owner;
         flat_set<account_name_type> inner_posting;
         std::vector< authority >    inner_other;

         operation_get_required_authorities( outer_o, outer_active, outer_owner, outer_posting, outer_other );

         for( const CustomOperationType& inner_o : custom_operations )
         {
            operation_validate( inner_o );
            operation_get_required_authorities( inner_o, inner_active, inner_owner, inner_posting, inner_other );
         }

         FC_ASSERT( inner_owner == outer_owner );
         FC_ASSERT( inner_active == outer_active );
         FC_ASSERT( inner_posting == outer_posting );
         FC_ASSERT( inner_other == outer_other );

         for( const CustomOperationType& inner_o : custom_operations )
         {
            // gcc errors if this-> is not here
            // error message is "declarations in dependent base are not found by unqualified lookup"
            this->get_evaluator( inner_o ).apply( inner_o );
         }

         plugin_session.squash();
      }

      virtual void apply( const protocol::custom_json_operation& outer_o ) override
      {
         try
         {
            std::vector< CustomOperationType > custom_operations;
            
            get_inner_operation( outer_o, custom_operations );
            apply_operations( custom_operations, operation( outer_o ) );
         } FC_CAPTURE_AND_RETHROW( (outer_o) )
      }

      virtual void apply( const protocol::custom_json_dapp_operation& outer_o ) override
      {
         try
         {
            std::vector< CustomOperationType > custom_operations;
            
            get_inner_operation( outer_o, custom_operations );
            apply_operations( custom_operations, operation( outer_o ) );
         } FC_CAPTURE_AND_RETHROW( (outer_o) )
      }

      virtual void apply( const protocol::custom_binary_operation& outer_o ) override
      {
         try
         {
            std::vector< CustomOperationType > custom_operations;

            get_inner_operation( outer_o, custom_operations );
            apply_operations( custom_operations, operation( outer_o ) );
         }
         FC_CAPTURE_AND_RETHROW( (outer_o) )
      }

      virtual std::shared_ptr< graphene::schema::abstract_schema > get_operation_schema() override
      {
         return graphene::schema::get_schema_for_type< CustomOperationType >();
      }

   private:
      void get_inner_operation( const protocol::custom_json_operation& outer_op, std::vector< CustomOperationType >& custom_operations )
      {
         fc::variant v = fc::json::from_string( outer_op.json );

         if( v.is_array() && v.size() > 0 && v.get_array()[0].is_array() )
         {
            from_variant( v, custom_operations );
         }
         else
         {
            custom_operations.emplace_back();
            from_variant( v, custom_operations[0] );
         }
      }

      void get_inner_operation( const protocol::custom_json_dapp_operation& outer_op, std::vector< CustomOperationType >& custom_operations )
      {
         fc::variant v = fc::json::from_string( outer_op.json );

         if( v.is_array() && v.size() > 0 && v.get_array()[0].is_array() )
         {
            from_variant( v, custom_operations );
         }
         else
         {
            custom_operations.emplace_back();
            from_variant( v, custom_operations[0] );
         }
      }

      void get_inner_operation( const protocol::custom_binary_operation& outer_op, std::vector< CustomOperationType >& custom_operations )
      {
         try
         {
            std::vector< CustomOperationType > inner_operations = fc::raw::unpack< vector< CustomOperationType > >( outer_op.data );
            auto itr = inner_operations.begin();
            while( itr != inner_operations.end() )
            {
               custom_operations.push_back( *itr );
               itr++;
            }
         }
         catch ( fc::exception& )
         {
            custom_operations.push_back( fc::raw::unpack< CustomOperationType >( outer_op.data ) );
         }
      }
};

} }
