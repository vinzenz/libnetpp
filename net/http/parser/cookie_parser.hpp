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
#ifndef GUARD_NET_HTTP_PARSER_COOKIE_PARSER_HPP_INCLUDED
#define GUARD_NET_HTTP_PARSER_COOKIE_PARSER_HPP_INCLUDED

namespace net
{
		class cookie_jar
		{			
		public:
			cookie_jar()
			{
				
			}
			
			cookie_jar(cookie_jar const & other)
			{
				
			}
			
			~cookie_jar()
			{
				
			}
			
			cookie_jar & operator=(cookie_jar other)
			{
				swap(other);
				return *this;
			}
			
			void swap(cookie_jar & )
			{
				
			}
			
			
		};
		
		template<typename Tag>
		class basic_cookie_parser
		{
		public:
			basic_cookie_parser()
			{
				
			}
			
			bool parse(cookie_jar & jar, basic_message<Tag> & message)
			{
				typedef typename header_collection_traits<Tag>::type::const_iterator iterator_t;
				typedef std::pair<iterator_t, iterator_t> iterator_range_t;
					
				iterator_range_t range = message.headers().equal_range("Set-Cookie");
				for(iterator_t iter = range.first; iter != range.second; ++iter)
				{
					do_parse(jar, iter->second);
				}
				return true;
			}
			
		private:
			void do_parse(cookie_jar & jar, std::string const & data)
			{
				cookie c;
				
				size_t pos = data.find("=");
				if(pos == std::string::npos)
				{
					return;
				}
				
				c.name() = data.substring(0,pos);
				++pos;
				if(data.size() <= pos)
				{
					return;
				}
				
				if(data[pos] == '"')
				{
					size_t last = pos + 1;
					pos = data.find('"', last);
					if(pos == std::string::npos)
					{
						
					}
					c.value() = data.substring(last, pos - last);
					pos = data.find_first_not_of("; \t\n", pos);
				}
				else
				{
					size_t last = pos;
					pos = data.find_first_not_of(" ;\r\n\t", last);
					c.value() = data.substring(last, pos - last);
				}
				
				
				
			}
		};
	}
}

#endif //GUARD_NET_HTTP_PARSER_COOKIE_PARSER_HPP_INCLUDED
