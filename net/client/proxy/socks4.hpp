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
#ifndef GUARD_NET_CLIENT_PROXY_SOCKS4_HPP_INCLUDED
#define GUARD_NET_CLIENT_PROXY_SOCKS4_HPP_INCLUDED

#include <net/client/proxy_socket.hpp>

namespace net
{
	template<typename Tag>
	struct socks4_proxy
		: implements_proxy<Tag>
	{
		typedef implements_proxy<Tag>					base_type;
		typedef typename base_type::service_type		service_type;
		typedef typename base_type::endpoint_type		endpoint_type;
		typedef typename base_type::connected_handler	connected_handler;
		
		socks4_proxy(service_type & service)
			: base_type(service)
		{}

		virtual void on_async_connected(
			proxy_socket<Tag> &	socket, 
			endpoint_type const & endpoint,
			connected_handler connected
		)
		{
			std::cout << "Connected to proxy..." << std::endl;
		}

		virtual error_code on_connected(
			proxy_socket<Tag> & socket, 
			endpoint_type const & endpoint, 
			error_code & ec
		)
		{
			std::cout << "Connected to proxy..." << std::endl;
			return ec;
		}		

		struct request
		{
			boost::uint8_t	version;
			boost::uint8_t	command;
			boost::uint16_t destination_port;
			boost::uint32_t destination_address;
			boost::uint8_t	end_marker;
		};

		union request_conv
		{
			request detail;
			boost::array<boost::uint8_t, sizeof(request)> bytes;
		};

		request_conv build_request(endpoint_type ep, error_code & ec)
		{
			request_conv rc = request_conv(); 
			if(!ep.address().is_v4())
			{
				ec = boost::system::error_code(boost::asio::error::address_family_not_supported);
				return rc;
			}
			rc.detail.version = 4;
			rc.detail.command = 1;
			rc.detail.destination_port =  ep.address().data().v4.sin_port;
			rc.detail.destination_address = ep.address().data().v4.sin_addr;
			rc.detail.end_marker = 0;
			return rc;
		}

	};
}

#endif //GUARD_NET_CLIENT_PROXY_SOCKS4_HPP_INCLUDED

