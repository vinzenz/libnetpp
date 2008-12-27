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
#include <fstream>
#include <iostream>
#include <iterator>
#include <net/http.hpp>
#include <net/http/parser/header_parser.hpp>
#include <net/http/parser/content_parser.hpp>
#include <boost/foreach.hpp>
#include <net/http/cookie.hpp>
#include <net/http/parser/cookie_parser.hpp>

int main( int argc, char **argv )
{
	std::ifstream inp("../data/3.dat", std::ios::binary);
	inp >> std::noskipws;
	std::istream_iterator<char> iter(inp);
	std::istream_iterator<char> end;
	net::http::basic_header_parser<net::http::message_tag, false> header_parser;
	net::http::basic_response<net::http::message_tag> message;
	std::cout << "Parse Success: " << std::boolalpha << header_parser.parse(iter , end, message)  << std::endl;
	std::cout << "Parser state: " << std::boolalpha << header_parser.valid() << std::endl;
	std::cout << "Status Code: " << message.status_code() << std::endl;
	std::cout << "Message: <" << message.status_message() << ">" << std::endl;
	std::cout << "Version: " <<  unsigned(message.version().first) << "." << unsigned(message.version().second) << std::endl;
	std::cout << "Header Data: " << std::endl;
	typedef std::pair<std::string, std::string> pair_type;
	BOOST_FOREACH(pair_type const & p, message.headers())
	{
		std::cout << "\t" << p.first << " : " << p.second << std::endl;
	}
	std::string data(iter, end);
	std::string::iterator striter = data.begin();
	net::http::basic_chunked_content_parser<net::http::message_tag> chunk_parser;
	boost::tribool result = chunk_parser.parse(striter, data.end(), message) ;
	std::cout << "Chunk Parse Success: " << std::boolalpha << result << std::endl;
	std::cout << data.end() - striter << std::endl;;
	std::cout << unsigned(*striter) << " = <" << *striter<< ">" << std::endl;
    net::http::basic_cookie_parser<net::http::message_tag> parser;
    net::http::cookie_jar jar;
    message.headers().insert(std::make_pair("Set-Cookie", "name=\"value;a;b;c;d=e;f;g\"hijasd\"; Comment=\"comment\"; Domain=\"appinf.com\"; Path=\"/\"; Max-Age=\"100\"; HttpOnly; Version=\"0\""));
    parser.parse(jar, message);
	return 0;
}
