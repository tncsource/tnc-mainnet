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

#include <sigmaengine/app/plugin.hpp>
#include <sigmaengine/chain/sigmaengine_objects.hpp>

#include <boost/asio/high_resolution_timer.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/asio.hpp>

#ifndef TXT_TEXT_GEN_PLUGIN_NAME
#define TXT_TEXT_GEN_PLUGIN_NAME "txt_test_gen"
#endif

namespace sigmaengine { namespace txt_test_gen {
namespace detail { struct txt_test_gen_plugin_impl; }

using app::application;
using namespace sigmaengine::chain;

class txt_test_gen_plugin : public sigmaengine::app::plugin
{
   std::unique_ptr<detail::txt_test_gen_plugin_impl> my;
public:
   txt_test_gen_plugin( application* app );
   virtual ~txt_test_gen_plugin();

   std::string plugin_name()const override { return TXT_TEXT_GEN_PLUGIN_NAME; }
   virtual void plugin_set_program_options(boost::program_options::options_description&,
                                           boost::program_options::options_description& cfg) override;
   virtual void plugin_initialize(const boost::program_options::variables_map& options) override;
   virtual void plugin_startup() override;
   virtual void plugin_shutdown() override;

   void arm_timer(boost::asio::high_resolution_timer::time_point s);
   void send_transaction(std::function<void(const fc::exception_ptr&)> next, uint64_t nonce_prefix);
   void start_generation(uint16_t thread, unsigned repeat, uint16_t count, string accountA, string accountB, string key);
   void stop_generation();

};

} } //sigmaengine::account_history

