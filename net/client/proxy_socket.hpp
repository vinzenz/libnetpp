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
#ifndef GUARD_NET_CLIENT_PROXY_SOCKET_HPP_INCLUDED
#define GUARD_NET_CLIENT_PROXY_SOCKET_HPP_INCLUDED

#include <boost/asio/ip/tcp.hpp>
#include <net/client/proxy/base.hpp>

namespace net
{
	template<typename Tag>
	struct proxy_socket
		: boost::asio::ip::tcp::socket
	{
		typedef boost::asio::ip::tcp::socket			base_type;
        typedef base_type                               next_layer_type;
		typedef boost::asio::io_service					service_type;
		typedef boost::asio::ip::tcp::endpoint			endpoint_type;
		typedef boost::system::error_code				error_code;
		typedef typename proxy_base<Tag>::self_ptr		proxy_base_ptr;

		explicit proxy_socket(service_type & service)
			: base_type(service)
			, proxy_ptr_(new proxy_base<Tag>(service))
		{}

		void set_proxy(proxy_base_ptr proxy)
		{
			if(proxy)
			{
				proxy_ptr_ = proxy;
			}
			else
			{
				proxy_ptr_.reset(new proxy_base<Tag>(this->get_io_service())); // disables the proxy
			}
		}

		boost::system::error_code connect(endpoint_type const & peer_endpoint, boost::system::error_code & ec)
		{	
			if(!ec)
			{
				proxy_ptr_->connect(*this, peer_endpoint, ec);
			}
			return ec;
		}

#ifndef BOOST_NO_EXCEPTIONS
		void connect(endpoint_type const & peer_endpoint)
		{
			error_code ec;
			connect(peer_endpoint, ec);
			boost::asio::detail::throw_error(ec);
		}
#endif //#ifndef BOOST_NO_EXCEPTIONS

		template <class Handler>
		void async_connect(endpoint_type const & endpoint, Handler const & handler)
		{
			proxy_ptr_->async_connect(
				*this,
				endpoint,
				handler
			);
		}

		void cancel(error_code & ec)
		{
			proxy_ptr_->cancel();
			base_type::cancel(ec);
		}

#ifndef BOOST_NO_EXCEPTIONS
		void cancel()
		{
			proxy_ptr_->cancel();
			base_type::cancel();
		}
#endif //#ifndef BOOST_NO_EXCEPTIONS

		void close(error_code & ec)
		{
			proxy_ptr_->cancel();
			base_type::cancel(ec);
		}

#ifndef BOOST_NO_EXCEPTIONS
		void close()
		{
			proxy_ptr_->cancel();
			base_type::close();
		}
#endif //#ifndef BOOST_NO_EXCEPTIONS

        next_layer_type & next_layer()
        {
            return *static_cast<next_layer_type*>(this);
        }

        next_layer_type const & next_layer() const
        {
            return *static_cast<next_layer_type const*>(this);
        }
	protected:
		proxy_base_ptr proxy_ptr_;
	};	
}

#endif //GUARD_NET_CLIENT_PROXY_SOCKET_HPP_INCLUDED
