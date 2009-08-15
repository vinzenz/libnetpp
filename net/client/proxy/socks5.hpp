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
#ifndef GUARD_NET_CLIENT_PROXY_SOCKS5_HPP_INCLUDED
#define GUARD_NET_CLIENT_PROXY_SOCKS5_HPP_INCLUDED

#include <net/client/proxy_socket.hpp>

namespace net
{
	template<typename Tag>
	struct socks5_proxy
		: implements_proxy<Tag>
	{
		typedef implements_proxy<Tag>					base_type;
		typedef typename base_type::service_type		service_type;
		typedef typename base_type::endpoint_type		endpoint_type;
		typedef typename base_type::connected_handler	connected_handler;
		
		union request_t
		{
			boost::uint8_t version;
			boost::uint8_t cmd_or_reply;
			boost::uint8_t reserved;
			boost::uint8_t address_type;			
		};

		union ipv4_address
		{
			struct
			{
				boost::array< boost::uint8_t, 4> address;
				boost::uint16_t port;
			} details;
			boost::array<boost::uint8_t, 6> bytes;
		};

		union ipv6_address
		{
			struct  
			{
				boost::array<boost::uint8_t, 16> address;
				boost::uint16_t port;
			} details;
			boost::array<boost::uint8_t, 18> bytes;
		};


		struct session
		{
			session(proxy_socket<Tag> & socket, 
					endpoint_type const & ep,
					connected_handler const & connected = connected_handler())
				: data_buffer()
				, socket_ref(boost::ref(socket))
				, endpoint(ep)
				, handler(connected)
			{}

			boost::array<boost::uint8_t, 0x1000> data_buffer;
			boost::reference_wrapper< proxy_socket<Tag> > socket_ref;
			endpoint_type endpoint;
			connected_handler handler;			
			
			request_t	 auth_request;
			request_t	 connect_request;
			ipv6_address ipv6;
			ipv4_address ipv4;
			boost::asio::mutable_buffer auth_request_buffer; 
			boost::array<boost::asio::mutable_buffer, 2> request_buffers; 

			void build_request()
			{
				auth_request.version			= 0x05; // Version 5
				auth_request.cmd_or_reply		= 0x01; // 1 Method supported
				auth_request.reserved			= 0x00; // No Auth Method

				connect_request.version			= 0x05;
				connect_request.cmd_or_reply	= 0x01; // CONNECT
				connect_request.reserved		= 0;
				if(endpoint.address().is_v4())
				{
					connect_request.address_type = 0x01; // IPv4 Address
					ipv4.details.port    = ::htons(endpoint.port());
					ipv4.details.address = endpoint.address().to_v4().to_bytes();
					request_buffers[1] = boost::asio::buffer(ipv4.bytes);
				}
				else
				{
					connect_request.address_type = 0x04; // IPv6 Address
					ipv6.details.port    = ::htons(endpoint.port());
					ipv6.details.address = endpoint.address().to_v6().to_bytes();
					request_buffers[1] = boost::asio::buffer(ipv6.bytes);
				}
				request_buffers[0] = boost::asio::buffer(&connect_request, 4);
				auth_request_buffer = boost::asio::buffer(&auth_request, 3);
			}
		};

		typedef boost::shared_ptr<session> session_ptr;

		socks5_proxy(service_type & service)
			: base_type(service)
		{}

		virtual void on_async_connected(
			proxy_socket<Tag> &	socket, 
			endpoint_type const & endpoint,
			connected_handler connected
		)
		{
			session_ptr sess(new session(socket, endpoint, connected));
			sess->build_request();
			boost::asio::async_write(
				sess->socket_ref.get(),
				boost::asio::buffer(sess->data_buffer),
				boost::bind(
					&socks5_proxy::on_async_request_sent,
					this,
					boost::asio::placeholders::error,
					sess
				)
			);
		}

		void on_async_request_sent(
			error_code const & ec,
			session_ptr sess
		)
		{
			if(!ec)
			{
				boost::asio::async_read(
					sess->socket_ref.get(),
					boost::asio::buffer(sess->data_buffer),
					boost::asio::transfer_at_least(1),
					boost::bind(
						&socks5_proxy::on_async_response,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred,
						sess
					)
				);
			}
			else
			{
				sess->handler(ec);
			}
		}

		error_code translate_socks5_reply(boost::uint8_t reply)
		{
			switch(reply)
			{
			case 0:
				return error_code(); // Success
			case 1:
				// SOCKS failure
				return error_code(boost::asio::error::fault);
			case 2:
				// connection not allowed by rule set
				return error_code(boost::asio::error::access_denied);
			case 3:
				// Network unreachable
				return error_code(boost::asio::error::network_unreachable);
			case 4:
				// Host unreachable
				return error_code(boost::asio::error::host_unreachable);
			case 5:
				// Connection refused
				return error_code(boost::asio::error::connection_refused);
			case 6:
				// TTL expired
				return error_code(boost::asio::error::timed_out);
			case 7:
				// Command not supported
				return error_code(boost::asio::error::operation_not_supported);
			case 8:
				// Address type not supported
				return error_code(boost::asio::error::address_family_not_supported);
			default:
				break;
			}
			return error_code(boost::asio::error::service_not_found);
		}

		void on_async_response(
			error_code const & ec,
			size_t bytes_read,
			session_ptr sess
		)
		{
			error_code res = ec;
			if(!res)
			{
				res = check_response(*sess, bytes_read);
			}

			if(res)
			{
				sess->socket_ref.get().close();
			}
			sess->handler(res);
		}

		error_code check_response(session & sess, size_t bytes_read)
		{
			error_code ec;
			if(bytes_read > 1)
			{
				return translate_socks5_reply(sess.data_buffer[3]);
			}
			else
			{
				return error_code(boost::asio::error::connection_reset);
			}
			return ec;
		}

		virtual error_code on_connected(
			proxy_socket<Tag> & socket, 
			endpoint_type const & endpoint,
			error_code & ec
		)
		{
			if(!ec)
			{
				session sess(socket, endpoint);
				// Handshake:
				boost::uint8_t buffer[] = {5, 1, 0}; // SOCKSv5, 1 Auth Method, No Auth
				boost::asio::write(socket, boost::asio::buffer(buffer), boost::asio::transfer_all(), ec);
				size_t read = boost::asio::read(socket, boost::asio::buffer(buffer, 2), boost::asio::transfer_all(), ec);
				if(read == 2)
				{
					if(buffer[1] == 0)
					{
						boost::array<boost::uint8_t, 22> connection_buffer;
						size_t buffer_size = 10;
						boost::uint16_t port = ::htons(endpoint.port());
						if(endpoint.address().is_v4())
						{
							buffer_size = 10;
							memcpy(&connection_buffer[4], endpoint.address().to_v4().to_bytes().data(), 4);
							memcpy(&connection_buffer[8], &port, 2);
						}
						else
						{
							buffer_size = 22;
							memcpy(&connection_buffer[4], endpoint.address().to_v6().to_bytes().data(), 16);
							memcpy(&connection_buffer[20], &port, 2);
						}

						boost::asio::write(socket, boost::asio::buffer(connection_buffer.data(), buffer_size), boost::asio::transfer_all(), ec);
						boost::array< boost::uint8_t, 0x1000> response_buffer;
						size_t cnt = boost::asio::read(socket, boost::asio::buffer(response_buffer), boost::asio::transfer_at_least(1), ec);
						if(cnt)
						{
							return error_code();
						}
					}
				}
			}

			if(!ec)
			{
				socket.close();
			}
			return ec;
		}
	};
}

#endif //GUARD_NET_CLIENT_PROXY_SOCKS5_HPP_INCLUDED


