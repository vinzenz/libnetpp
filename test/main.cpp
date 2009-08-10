/*
 * Copyright (c) 2008 by Vinzenz Feenstra
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

typedef net::basic_client<net::default_tag> client;
typedef net::basic_client<net::default_tag>::socket_type socket_type;

char const * REQUEST = 
"GET / HTTP/1.1\r\n"
"Host: www.google.com\r\n"
"\r\n";

typedef boost::array<char, 0x10000> buffer_t;

void response_received(socket_type * s, buffer_t * buffer, boost::system::error_code const & ec, size_t bytes_received)
{
    std::cout << "Received:\n";    
//    std::cout << buffer->data();
}

void request_sent(socket_type * s, boost::system::error_code const & ec, size_t bytes_sent)
{
    std::cout << "Request sent waiting for reply:\n";    
    buffer_t * buf = new buffer_t();
    boost::asio::async_read(
        *s,
        boost::asio::buffer(*buf),
        boost::bind(
            response_received,
            s,
            buf,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}


void send_request(socket_type * s)
{
    std::cout << "Sending request:\n";    
    boost::asio::async_write(*s, boost::asio::buffer(REQUEST), boost::bind(request_sent, s, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}


void say(socket_type * s, boost::system::error_code const & ec, std::string const & name)
{
    if(ec)
    {
        std::cout << "[" << name << "]: Establishing connection failed: " << ec << " Message: " << ec.message() << std::endl;
    }
    else
    {
        std::cout << "[" << name << "]: Connection established" << std::endl;
        send_request(s);
    }
}

int main()
{
    boost::asio::io_service service;

    boost::asio::ssl::context ctx(service, boost::asio::ssl::context::sslv23);
    ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);
    ctx.load_verify_file("/code/libnetpp/ca.pem");

    client c(service);
    c.connect("www.google.com","80", boost::bind(say, &c.socket(), _1, "Plain"));
    

    client ssl_c(service, ctx);
    ssl_c.connect("www.google.com","443", boost::bind(say, &ssl_c.socket(), _1, "SSL"));

    service.run();
    return EXIT_SUCCESS;
}
