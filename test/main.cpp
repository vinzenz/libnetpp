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
#include <iostream>
#include <net/client/client.hpp>
#include <net/detail/tags.hpp>

#include <net/client/proxy/http.hpp>
#include <net/client/proxy/socks4.hpp>
#if 0
#include <net/client/proxy/socks5.hpp>
#endif

typedef net::basic_client<net::default_tag> client;
typedef net::socket_adapter<net::default_tag> socket_type;
char REQUEST[] = 
"GET / HTTP/1.0\r\n"
"\r\n";

char HTTPS_REQUEST[] = 
"GET / HTTP/1.1\r\n"
"Host: www.google.com:443\r\n"
"\r\n";

typedef boost::array<char, 0x10000> buffer_t;

void response_received(socket_type & s, buffer_t * buffer, boost::system::error_code const & ec, size_t bytes_received, std::string const & name)
{    
	std::cout << "[" << name << "]: Received ("
			  << "Error code: " << ec << " message: " << ec.message() << "):\n";
    if(bytes_received == 0)
	{
		std::cout << "NO DATA\n\n";
        return;
    }
	std::cout << buffer->data();
}

void request_sent(socket_type & s, boost::system::error_code const & ec, size_t bytes_sent, std::string const & name)
{
	if(ec)
	{
		std::cout << "Failed sending request: " << ec << " Message: " << ec.message() << std::endl;
		return;
	}
	std::cout << "[" << name << "]: Request sent waiting for reply:\n";    
	buffer_t * buf = new buffer_t();
	boost::asio::async_read(
		s,
		boost::asio::buffer(*buf),
		boost::asio::transfer_at_least(1),
		boost::bind(
			response_received,
			boost::ref(s),
			buf,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			name
		)
	);
}


void send_request(socket_type & s, std::string const & name)
{
	std::cout << "[" << name << "]: Sending request:\n";    
	boost::asio::async_write(
		s, 
		(name == "SSL" ? boost::asio::buffer(HTTPS_REQUEST) : boost::asio::buffer(REQUEST)), 
		boost::bind(
			request_sent, 
			boost::ref(s), 
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred,
			name
		)
	);
}


void say(socket_type & s, boost::system::error_code const & ec, std::string const & name)
{
	if(ec)
	{
		std::cout << "[" << name << "]: Establishing connection failed: " << ec << " Message: " << ec.message() << std::endl;
	}
	else
	{
		std::cout << "[" << name << "]: Connection established" << std::endl;
		send_request(s, name);
	}
}

int main(int argc, char const **argv)
{
	try
	{
		boost::asio::io_service service;

		boost::asio::ssl::context ctx(service, boost::asio::ssl::context::sslv23);
		ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);
		if(argc > 1)		
		{
			ctx.load_verify_file(argv[1]);
		}

		// SOCKS4 and 5 Proxy:
		client::proxy_base_ptr socks4_proxy_ptr(new net::socks4_proxy<net::default_tag>(service));
		socks4_proxy_ptr->set_server( argc > 2 ? argv[2] : "59.174.25.245" , argc > 3 ? argv[3] : "1080");
		
		// HTTP Connect Proxy:
		//		proxy_ptr->set_server("67.69.254.249", "80");

		client c(service);
		//c.set_proxy(socks4_proxy_ptr);
		c.async_connect("www.google.com","80", boost::bind(say, boost::ref(c.socket()), _1, "Plain"));

		client::proxy_base_ptr empty;
		client ssl_c(service, ctx);
		ssl_c.set_proxy(socks4_proxy_ptr);

		service.run();
	}
	catch(boost::system::system_error const & e)
	{
		std::cout << e.code() << " Message: " << e.code().message() << std::endl;
	}
	catch (std::exception const & e)
	{
		std::cout << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}

