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
#include <net/client/utils/buffer.hpp>

namespace net
{
	template<typename Tag>
	struct socks5_proxy
		: implements_proxy<Tag>
	{
		typedef implements_proxy<Tag>					base_type;
		typedef typename base_type::error_code			error_code;
		typedef typename base_type::service_type		service_type;
		typedef typename base_type::endpoint_type		endpoint_type;
		typedef typename base_type::connected_handler	connected_handler;

		struct session
		{
			session(proxy_socket<Tag> & socket, 
					endpoint_type const & ep,
					connected_handler const & connected = connected_handler())
				: socket_ref(boost::ref(socket))
				, endpoint(ep)
				, handler(connected)
				, auth_buffer()
				, connection_buffer()
				, response_buffer()
				, auth(boost::asio::buffer(auth_buffer))
				, connection(boost::asio::buffer(connection_buffer))
			{
				memset(auth_buffer.data(),0, auth_buffer.size());
				memset(connection_buffer.data(), 0, connection_buffer.size());
				memset(response_buffer.data(), 0, response_buffer.size());
				build_requests();
			}

			boost::reference_wrapper< proxy_socket<Tag> > socket_ref;
			endpoint_type endpoint;
			connected_handler handler;			

			boost::array<boost::uint8_t, 3>	  auth_buffer;
			boost::array<boost::uint8_t, 22>  connection_buffer;					
			boost::array< boost::uint8_t, 22> response_buffer;						
			boost::asio::mutable_buffer auth;
			boost::asio::mutable_buffer connection;

			void build_requests()
			{
				util::adapt_unchecked(auth_buffer)
					.writeu8(0x05)
					.writeu8(0x01)
					.writeu8(0x00);
				auth = boost::asio::buffer(auth_buffer);

				util::adapt_unchecked(connection_buffer)
					.writeu8(0x05)
					.writeu8(0x01)
					.writeu8(0x00)
					.writeu8(endpoint.address().is_v4() ? 0x01 : 0x04 )
					.write(endpoint.address())
					.writeu16(endpoint.port());
				connection = boost::asio::buffer(connection_buffer.data(), endpoint.address().is_v4() ? 10 : 22);
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
			boost::asio::async_write
			(
				sess->socket_ref.get(),
				boost::asio::buffer(sess->auth),
				boost::bind
				(
					&socks5_proxy::on_async_auth_request_sent,
					this,
					boost::asio::placeholders::error,
					sess
				)
			);
		}

		void on_async_auth_request_sent(
			error_code const & ec,
			session_ptr sess
		)
		{
			if(!ec)
			{
				boost::asio::async_read(
					sess->socket_ref.get(),
					boost::asio::buffer(sess->auth_buffer.data(), 2),
					boost::bind
					(
						&socks5_proxy<Tag>::on_async_auth_response_received,
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

		void on_async_auth_response_received(
			error_code const & ec,
			size_t,
			session_ptr sess
		)
		{
			if(!ec)
			{
				if(sess->auth_buffer[0] == 0x05 && sess->auth_buffer[1] == 0x00)
				{
					boost::asio::async_write
					(
						sess->socket_ref.get(),
						boost::asio::buffer(sess->connection),
						boost::bind
						(
							&socks5_proxy::on_async_connection_request_sent,
							this,
							boost::asio::placeholders::error,
							sess
						)
					);
				}
				else
				{
					sess->handler(error_code(boost::asio::error::access_denied));
				}
			}
			else
			{
				sess->handler(ec);
			}
		}

		void on_async_connection_request_sent(
			error_code const & ec,
			session_ptr sess
		)
		{
			if(!ec)
			{
				boost::asio::async_read
				(
					sess->socket_ref.get(),
					boost::asio::buffer(sess->connection),
					boost::asio::transfer_at_least(10),
					boost::bind
					(
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
				std::cout << "Connection via proxy established\n\n";
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
				return translate_socks5_reply(sess.response_buffer[3]);
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

				boost::asio::write
				(
					socket, 
					boost::asio::buffer(sess.auth), 
					boost::asio::transfer_all(), 
					ec
				);

				size_t read = boost::asio::read
				(
					socket, 
					boost::asio::buffer
					(
						sess.auth_buffer.data(), 
						2
					), 
					boost::asio::transfer_all(),
					ec
				);

				if(read == 2)
				{
					boost::uint8_t version = 0;
					boost::uint8_t method  = 0;

					util::adapt_unchecked(sess.auth_buffer)
						.readu8(version)
						.readu8(method);

					if(version == 5 && method == 0)
					{

						boost::asio::write
						(
							socket, 
							boost::asio::buffer(sess.connection),
							boost::asio::transfer_all(), 
							ec
						);

						size_t cnt = boost::asio::read
						(
							socket, 
							boost::asio::buffer(sess.response_buffer), 
							boost::asio::transfer_at_least(1), 
							ec
						);

						// Check: 
						// - We have version 5
						// - the read size = 10 if addr type = 0x01 (ipv4)
						// - the read size = 22 if addr type = 0x04 (ipv6)
						if( sess.response_buffer[0] == 5 
						&&  ( (cnt == 10 &&	sess.response_buffer[3] == 0x01) 
							||(cnt == 22 &&	sess.response_buffer[3] == 0x04)
						))
						{
							// Translate the repsonse
							return translate_socks5_reply(sess.response_buffer[1]);
						}
						
						// Most likely because of invalid protocol
						ec = error_code(boost::asio::error::connection_aborted);
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


