#pragma once

#include <sigmaengine/protocol/authority.hpp>
#include <sigmaengine/protocol/sigmaengine_operations.hpp>

#include <sigmaengine/chain/sigmaengine_object_types.hpp>
#include <sigmaengine/chain/bobserver_objects.hpp>

#include <boost/multi_index/composite_key.hpp>


namespace sigmaengine { namespace chain {
   
   struct strcmp_less
   {
      bool operator()( const shared_string& a, const shared_string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      bool operator()( const shared_string& a, const string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      bool operator()( const string& a, const shared_string& b )const
      {
         return less( a.c_str(), b.c_str() );
      }

      private:
         inline bool less( const char* a, const char* b )const
         {
            return std::strcmp( a, b ) < 0;
         }
   };

   class comment_object : public object < comment_object_type, comment_object >
   {
      comment_object() = delete;

      public:
         template< typename Constructor, typename Allocator >
         comment_object( Constructor&& c, allocator< Allocator > a )
            :category( a ), parent_permlink( a ), permlink( a ), title( a ), body( a ), json_metadata( a )
         {
            c( *this );
         }

         id_type           id;

         shared_string     category;
         account_name_type parent_author;
         shared_string     parent_permlink;
         account_name_type author;
         shared_string     permlink;

         int32_t           group_id = 0;   ///< Group id, this is 0 or greater than 0. However 0 is main feed else is group feed. 

         shared_string     title;
         shared_string     body;
         shared_string     json_metadata;
         time_point_sec    last_update;
         time_point_sec    created;
         time_point_sec    active; ///< the last time this post was "touched" by voting or reply

         uint16_t          depth = 0; ///< used to track max nested depth
         uint32_t          children = 0; ///< used to track the total number of children, grandchildren, etc...

         uint32_t          like_count = 0;
         uint32_t          dislike_count = 0;
         
         uint64_t          view_count = 0;

         id_type           root_comment;

         bool              allow_replies = true;      /// allows a post to disable replies.
         bool              allow_votes   = true;      /// allows a post to receive votes;
         bool              is_blocked = false;
   };
} } // sigmaengine::chain