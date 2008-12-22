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



int main( int argc, char **argv )
{
	std::ifstream inp("../data/1.dat", std::ios::binary);
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
	return 0;
}
