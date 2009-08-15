/*
 * Copyright (c) 2008,2009 by Vinzenz Feenstra
 * All rights reserved.
 *
 * - Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Neither the name of the Vinzenz Feenstra nor the names
 *   of its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef GUARD_NET_CLIENT_CLIENT_HPP_INCLUDED
#define GUARD_NET_CLIENT_CLIENT_HPP_INCLUDED

#include <net/client/socket_adapter.hpp>

namespace net
{
    template<typename Tag>
    struct basic_client
    {         
        typedef boost::shared_ptr< connection_base<Tag> >       connection_ptr;
        typedef socket_adapter<Tag>                             socket_type;
        typedef typename connection_base<Tag>::string_type      string_type;
        typedef typename connection_base<Tag>::callback         callback;   
        typedef typename connection_base<Tag>::service_type     service_type;
        typedef typename connection_base<Tag>::ssl_context_type ssl_context_type;
		typedef typename proxy_base<Tag>::self_ptr				proxy_base_ptr;

        basic_client(service_type & service)
        : adapter_(connection_ptr(new connection<Tag>(service)), false)
        {}

        basic_client(service_type & service, ssl_context_type & context)
        : adapter_(connection_ptr(new ssl_connection<Tag>(service, context)), true)
        {}

        ~basic_client()
        {}            

		void set_proxy(proxy_base_ptr ptr)
		{
			adapter_.set_proxy(ptr);
		}

		boost::system::error_code connect(string_type const & server, string_type const & port, boost::system::error_code & ec)
		{
			return adapter_.base().connect(server, port, ec);
		}

        void async_connect(string_type const & server, string_type const & port, callback cb)
        {
            adapter_.base().async_connect(server, port, cb);
        }        

        socket_adapter<Tag> & socket()
        {
            return adapter_;
        }

    protected:
        socket_adapter<Tag> adapter_;
    };
}


#endif //GUARD_NET_CLIENT_CLIENT_HPP_INCLUDED

