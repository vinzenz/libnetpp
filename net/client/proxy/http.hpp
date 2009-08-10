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
#ifndef GUARD_NET_CLIENT_PROXY_HTTP_HPP_INCLUDED
#define GUARD_NET_CLIENT_PROXY_HTTP_HPP_INCLUDED

#include <net/client/proxy_socket.hpp>

namespace net
{
	template<typename Tag>
	struct http_proxy
		: proxy_base<Tag>
	{
		typedef proxy_base::service_type service_type;

		http_proxy(service_type & service)
			: proxy_base(service)
		{}

		// "CONNECT %1%:%2% HTTP/%3%.%4%\r\n"
		// "Host: %5%\r\n"
		// "Proxy-Authorization: %6%\r\n"
		// "User-Agent: %7\r\n"
		// "Proxy-Connection: %8\r\n"
	};
}

#endif //GUARD_NET_CLIENT_PROXY_HTTP_HPP_INCLUDED
