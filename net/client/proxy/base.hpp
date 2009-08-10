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
#ifndef GUARD_NET_CLIENT_PROXY_BASE_HPP_INCLUDED
#define GUARD_NET_CLIENT_PROXY_BASE_HPP_INCLUDED

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace net
{
	template<typename Tag>
	struct proxy_socket;
	
	template<typename Tag>
	struct proxy_base
	{		
		typedef boost::shared_ptr<proxy_base<Tag> >			self_ptr;
		typedef boost::asio::ip::tcp::resolver				resolver;
		typedef boost::asio::io_service						service_type;
		typedef boost::system::error_code					error_code;
		typedef boost::function< void(error_code & ec) >	connected_handler;
		typedef boost::function< void(connected_handler) >	proxy_connected_handler;
		typedef typename net::string_traits<Tag>::type		string_type;

		proxy_base(service_type & service)
			: resolver_(service)
		{}

		void set_server(string_type const & server, string_type const & port)
		{
			server_ = server;
			port_	= port;
		}

		virtual ~proxy_base()
		{}

		void cancel(error_code & ec)
		{
			ec = error_code();
			resolver_.cancel();
		}

#ifndef BOOST_NO_EXCEPTIONS
		void cancel()
		{
			error_code ec;
			cancel(ec);
			boost::asio::detail::throw_error(ec);
		}
#endif //#ifndef BOOST_NO_EXCEPTIONS

		virtual void connect(proxy_socket<Tag> &, error_code &)
		{
		}

		virtual void async_connect(
			proxy_socket<Tag> &, 
			error_code &, 
			connected_handler		connected, 
			proxy_connected_handler proxy_connected
			)
		{
			proxy_connected(connected);
		}
	protected:
		resolver	resolver_;
		string_type server_;
		string_type port_;
	};
}

#endif //GUARD_NET_CLIENT_PROXY_BASE_HPP_INCLUDED
