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
#include <net/http/parser/header_parser.hpp>

namespace net
{
	// CONNECT %SERVER%:%PORT% HTTP/1.1
	// Host: %SERVER%:%PORT%
	// Proxy-Authorization: %AUTH% 
	// User-Agent: %UA%
	// Proxy-Connection: %PROXY_CONNECTION%
	// Connection: %PROXY_CONNECTION%

	template<typename Tag>
	struct http_proxy
		: implements_proxy<Tag>
	{
		typedef implements_proxy<Tag>					base_type;
		typedef typename base_type::error_code			error_code;
		typedef typename base_type::service_type		service_type;
		typedef typename base_type::endpoint_type		endpoint_type;
		typedef typename base_type::connected_handler	connected_handler;

		http_proxy(service_type & service)
			: base_type(service)
		{}

		virtual void on_async_connected(
			proxy_socket<Tag> &	socket, 
			endpoint_type const & endpoint,
			connected_handler connected
		)
		{
			boost::shared_ptr<std::string> string_ptr(new std::string(build_request(endpoint)));
   			boost::asio::async_write(
				socket,
				boost::asio::buffer(
					*string_ptr
				),
				boost::bind(
					&http_proxy::start_read_response,
					this,
					boost::asio::placeholders::error,
					boost::ref(socket),
					connected,
					string_ptr
				)
			);
		}

		std::string build_request(endpoint_type const & ep)
		{
			std::ostringstream request;
			request << "CONNECT " << ep.address().to_string() << ":" << ep.port() << " HTTP/1.0\r\n"
					<< "Proxy-Connection: Close\r\n"
					<< "\r\n"
					;			
			std::cout << "\n---------------------------------------\n"
					  << "Generated request:\n" 
					  << request.str()	<< "\n"
					  <<   "---------------------------------------\n\n";
			return request.str();
		}

        typedef boost::array<char, 0x1000> 	buffer_t;
		typedef boost::shared_ptr<buffer_t> buffer_ptr_t;

        typedef net::http::basic_header_parser<Tag, false>  parser_t;
        typedef net::http::basic_response<Tag>              message_type;
        typedef boost::shared_ptr<parser_t>                 parser_ptr_t;

		virtual void start_read_response(
			error_code const & ec,
			proxy_socket<Tag> &	socket, 
			connected_handler connected,
			boost::shared_ptr<std::string>
		)
		{
			buffer_ptr_t buf(new buffer_t());
            parser_ptr_t parser(new parser_t());
            setup_async_read(buf, parser, socket, connected);
		}

        void setup_async_read(
            buffer_ptr_t buf_ptr,
            parser_ptr_t parser_ptr,
			proxy_socket<Tag> &	socket, 
			connected_handler connected
        )
        {
			boost::asio::async_read(
				socket,
				boost::asio::buffer(*buf_ptr),
                boost::asio::transfer_at_least(1),
			    boost::bind(
                    &http_proxy::response_read,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    buf_ptr,
                    parser_ptr,
                    boost::ref(socket),
                    connected
                )	
			);
        }

		virtual void response_read(
			error_code const & ec,
            size_t bytes_read,
            buffer_ptr_t buf_ptr,
            parser_ptr_t parser_ptr,
			proxy_socket<Tag> &	socket, 
			connected_handler connected
		)
		{
            std::cout << std::string(buf_ptr->begin(), buf_ptr->begin() + bytes_read);        
            if(!ec)
            {
                message_type msg;
                char * begin = buf_ptr->begin();
                boost::tribool state = parser_ptr->parse( begin, begin + bytes_read, msg );
                if(state == boost::logic::indeterminate)
                {
                    setup_async_read(buf_ptr, parser_ptr, socket, connected);
                }
                else if(state == true)
                {
                    // Parsing succeeded, but it doesn't mean everything is ok    
                    // it just means the protocol is valid as we know it
                    // We've to check for the error code
                    if(msg.status_code() == 200)
                    {
                        // We're now connected - Go ahead and send your request :-)  
                        // => calling callback
                        std::cout << "Connection succeeded!" << std::endl;
                        connected(error_code());
                    }
                    else
                    {
                        std::cout << "Connection failed!" << std::endl;
                        //TODO: handle failures                    
                    }
                }
                else if(state == false)
                {
                    std::cout << "Parsing failed!" << std::endl;
                    //      Parsing failed
                    //TODO: handle failures                    
                }
            }
            else
            {
                std::cout << "Whatever failed! ec = " << ec << " Message: " << ec.message() << std::endl;
            }
		}

		virtual error_code on_connected(
			proxy_socket<Tag> & socket, 
			endpoint_type const & endpoint, 
			error_code & ec
		)
		{
            if(!ec)
            {
                boost::asio::write(
                    socket, 
                    boost::asio::buffer( 
                        build_request( endpoint ) 
                    ),
                    boost::asio::transfer_all(),
                    ec            
                );

                buffer_t buffer;
                parser_t parser;
                message_type response;
                while(!ec)
                {
                    size_t read_bytes = boost::asio::read(
                        socket,
                        boost::asio::buffer(
                            buffer
                        ),
                        boost::asio::transfer_at_least(1),
                        ec
                    );

                    std::cout << std::string(buffer.begin(), buffer.begin() + read_bytes);      
                    char * begin = buffer.begin();
                    boost::tribool result = parser.parse( begin, begin + read_bytes, response );

                    if(result == boost::logic::indeterminate)
                    {
                        // Not finished parsing yet
                        continue;
                    }
                    else
                    {
                        if(result == true)
                        {
                            if(response.status_code() == 200)
                            {
                                return (ec = error_code());                                    
                            }
                            // TODO: Not 200 response => e.g. Proxy authorization required
                        }
                        // Parsing failure
                        break;
                    }                    
                }
            }
            return ec;
		}
	};
}

#endif //GUARD_NET_CLIENT_PROXY_HTTP_HPP_INCLUDED
