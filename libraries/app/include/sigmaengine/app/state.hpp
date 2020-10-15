#pragma once
#include <sigmaengine/app/applied_operation.hpp>
#include <sigmaengine/app/sigmaengine_api_objects.hpp>

#include <sigmaengine/chain/global_property_object.hpp>
#include <sigmaengine/chain/account_object.hpp>
#include <sigmaengine/chain/sigmaengine_objects.hpp>

namespace sigmaengine { namespace app {
   using std::string;
   using std::vector;

   struct tag_name_index
   {
      vector< string > trending; /// pending payouts
   };

   /**
    *  Convert's vesting shares
    */
   struct extended_account : public account_api_obj
   {
      extended_account(){}
      extended_account( const account_object& a, const database& db ):account_api_obj( a, db ){}

      map<uint64_t,applied_operation>         transfer_history; /// transfer to/from vesting
      map<uint64_t,applied_operation>         other_history;
      set<string>                             bobserver_votes;
   };

   struct candle_stick {
      time_point_sec  open_time;
      uint32_t        period = 0;
      double          high = 0;
      double          low = 0;
      double          open = 0;
      double          close = 0;
      double          pia_volume = 0;
   };

   /**
    *  This struct is designed
    */
   struct state {
        string                            current_route;

        dynamic_global_property_api_obj   props;

        map< string, extended_account >   accounts;

        map< string, bobserver_api_obj >  bobservers;
        bobserver_schedule_api_obj        bobserver_schedule;
        string                            error;
   };

} }

FC_REFLECT_DERIVED( sigmaengine::app::extended_account,
                   (sigmaengine::app::account_api_obj),
                   (transfer_history)(other_history)(bobserver_votes))

FC_REFLECT( sigmaengine::app::tag_name_index, (trending) )
FC_REFLECT( sigmaengine::app::state, (current_route)(props)(accounts)(bobservers)(bobserver_schedule)(error) )

FC_REFLECT( sigmaengine::app::candle_stick, (open_time)(period)(high)(low)(open)(close)(pia_volume) );
