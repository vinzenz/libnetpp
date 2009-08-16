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

#if defined(WIN32) || defined(WIN64)
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
#endif

#include <net/client/proxy_socket.hpp>
#include <algorithm> 

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
		typedef typename base_type::error_code       	error_code;

		union request_t
		{
			struct
			{
				boost::uint8_t	version;
				boost::uint8_t	command;
				boost::uint16_t	destination_port;
				boost::array<boost::uint8_t, 4> destination_address;
				boost::uint8_t	end_marker;
			}
			detail;
			boost::array<boost::uint8_t, 9> bytes;
		};

		struct session
		{			
			session(proxy_socket<Tag> & socket)
				: data_buffer()
				, request(reinterpret_cast<request_t*>(&data_buffer[0]))
				, handler()
				, endpoint()
				, socket(boost::ref(socket))
			{				
			}

			boost::array<boost::uint8_t, 0x1000>			data_buffer;
			request_t									*	request;
			connected_handler								handler;
			endpoint_type									endpoint;
			boost::reference_wrapper<proxy_socket<Tag> >	socket;

			void dump_buffer(size_t size)
			{
				static char const hex_chars[17] = "0123456789ABCDEF";
				for(size_t i = 0; i < std::min(size, data_buffer.size()); ++i)
				{
					boost::uint8_t b = data_buffer[i];
					std::cout << " " << hex_chars[(b&0xF0)>>4] << hex_chars[b&0x0F];
				}
			}
		};

		typedef boost::shared_ptr<session> session_ptr;

		
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
            boost::system::error_code ec;       
            session_ptr sess(new session(socket));

			*(sess->request) = build_request(endpoint, ec);
            if(ec) // Something went wrong with build_request 
            {
                std::cout << "Something went wrong with build_request: " << ec << " Message: " << ec.message() << std::endl;
                connected(ec);
            }

			sess->handler = connected;
			sess->endpoint = endpoint;

			std::cout << "Attempt to write: " << sess->request->bytes.size() << " bytes: ";
			sess->dump_buffer(sess->request->bytes.size());
            std::cout << std::endl;

            boost::asio::async_write(
                sess->socket.get(),
                boost::asio::buffer(sess->request->bytes),				
                boost::bind(
                    &socks4_proxy::on_async_request_sent,
                    this,
                    boost::asio::placeholders::error,
					sess
                )
            );
		}

		virtual void on_async_request_sent(
            error_code const & ec,
			session_ptr sess
		)
        {
            if(!ec)
            {
                boost::asio::async_read(
                    sess->socket.get(),
                    boost::asio::buffer(sess->data_buffer),
					boost::asio::transfer_at_least(8),
                    boost::bind(
                        &socks4_proxy::on_async_response_received,
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

        
		virtual void on_async_response_received(
            error_code const & ec,
            size_t bytes_transferred,
			session_ptr sess
		)
        {
            std::cout << ec << " Response received: (" << bytes_transferred << ") ";
			sess->dump_buffer(bytes_transferred);
            std::cout << std::endl;

            if(!ec)
            {
                if(sess->data_buffer[1] == 0x5a)
                {
                    sess->handler(error_code());
                }
				else
				{
					sess->handler(error_code(boost::asio::error::connection_refused));
				}
            }
			else
			{
				sess->handler(ec);
			}
        }

		virtual error_code on_connected(
			proxy_socket<Tag> & socket, 
			endpoint_type const & endpoint, 
			error_code & ec
		)
		{			
			std::cout << "Connected to proxy..." << std::endl;

			request_t request = build_request(endpoint, ec);
			if(!ec)
			{
				std::cout << "Sending connection request to proxy..." << std::endl;
				socket.send( boost::asio::buffer(request.bytes), 0,  ec);
				if(!ec)
				{
					std::cout << "Reading response from  proxy..." << std::endl;
					boost::array<boost::uint8_t, 8> buffer;
					size_t bytes_read = socket.read_some(boost::asio::buffer(buffer), ec);
					if(!ec)
					{
						if(bytes_read == 8)
						{
							if(buffer[1] == 0x5a)
							{
								std::cout << "Connection to remote endpoint granted and established" << std::endl;
								return (ec = error_code());
							}
						}
						std::cout << "Connection to remote endpoint denied or invalid response" << std::endl;
						return (ec = error_code(boost::asio::error::connection_refused));
					}
				}
			}
			return ec;
		}		

		virtual request_t build_request(endpoint_type ep, error_code & ec)
		{
			request_t rc = request_t(); 
			if(!ep.address().is_v4())
			{
				ec = error_code(boost::asio::error::address_family_not_supported);
				return rc;
			}
			rc.detail.version = 4;
			rc.detail.command = 1;
			rc.detail.destination_port = ::htons(ep.port());
			rc.detail.destination_address = ep.address().to_v4().to_bytes();
			rc.detail.end_marker = 0;
			return rc;
		}

	};
}

#endif //GUARD_NET_CLIENT_PROXY_SOCKS4_HPP_INCLUDED

