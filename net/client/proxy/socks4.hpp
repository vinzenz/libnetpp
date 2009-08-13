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
		struct request
		{
			boost::uint8_t	version;
			boost::uint8_t	command;
			boost::uint16_t destination_port;
		    boost::array<boost::uint8_t, 4> destination_address;
			boost::uint8_t	end_marker;
		};

		union request_conv
		{
			request detail;
			boost::array<boost::uint8_t, 9> bytes;
		};


		typedef implements_proxy<Tag>					base_type;
		typedef typename base_type::service_type		service_type;
		typedef typename base_type::endpoint_type		endpoint_type;
		typedef typename base_type::connected_handler	connected_handler;
		typedef typename base_type::error_code       	error_code;
		
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
            boost::shared_ptr<request_conv> buffer(new request_conv(build_request(endpoint, ec)));

            if(ec) // Something went wrong with build_request 
            {
                std::cout << "Something went wrong with build_request: " << ec << " Message: " << ec.message() << std::endl;
                connected(ec);
                return;
            }
            static char const hex_chars[17] = "0123456789ABCDEF";
            std::cout << "Attempt to write: " << buffer->bytes.size() << " bytes: ";
            for(size_t i = 0; i < buffer->bytes.size(); ++i)
            {
                boost::uint8_t b = buffer->bytes[i];
                std::cout << " " << hex_chars[(b&0xF0)>>4] << hex_chars[b&0x0F];
            }
            std::cout << std::endl;

            boost::asio::async_write(
                socket,
                boost::asio::buffer(buffer->bytes),
                boost::bind(
                    &socks4_proxy::on_async_request_sent,
                    this,
                    buffer,
                    boost::asio::placeholders::error,
                    boost::ref(socket),
                    connected
                )
            );
		}

		virtual void on_async_request_sent(
            boost::shared_ptr<request_conv>,
            error_code const & ec,
			proxy_socket<Tag> &	socket, 
			connected_handler connected
		)
        {
            if(!ec)
            {
                boost::shared_ptr< boost::array<boost::uint8_t, 8> > buffer(new boost::array<boost::uint8_t, 8>());
                boost::asio::async_read(
                    socket,
                    boost::asio::buffer(*buffer),
                    boost::bind(
                        &socks4_proxy::on_async_response_received,
                        this,
                        buffer,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred,
                        boost::ref(socket),
                        connected
                    )
                );
            }
            else
            {
                connected(ec);
            }
        }

        
		virtual void on_async_response_received(
            boost::shared_ptr< boost::array<boost::uint8_t, 8> > response_buffer,
            error_code const & ec,
            size_t bytes_transferred,
			proxy_socket<Tag> &	socket, 
			connected_handler connected
		)
        {
            static char const hex_chars[17] = "0123456789ABCDEF";
            boost::uint8_t b = (*response_buffer)[1];
            std::cout << ec << " Response received: (" << bytes_transferred << ") ";
            for(size_t i = 0; i < response_buffer->size(); ++i)
            {
                boost::uint8_t b = (*response_buffer)[i];
                std::cout << " " << hex_chars[(b&0xF0)>>4] << hex_chars[b&0x0F];
            }
            std::cout << std::endl;

            if(!ec)
            {
                if((*response_buffer)[1] == 0x5a)
                {
                    connected(error_code());
                }
                //TODO: Handle forbidden or other failures
            }
            connected(ec);
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

		virtual request_conv build_request(endpoint_type ep, error_code & ec)
		{
			request_conv rc = request_conv(); 
			if(!ep.address().is_v4())
			{
				ec = boost::system::error_code(boost::asio::error::address_family_not_supported);
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

