/*
 * Copyright (c) 2008, 2009 by Vinzenz Feenstra
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
#ifndef GUARD_NET_HTTP_DETAIL_TRAITS_HPP_INCLUDED
#define GUARD_NET_HTTP_DETAIL_TRAITS_HPP_INCLUDED

#include <net/http/detail/tags.hpp>
#include <net/detail/traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/cstdint.hpp>

namespace net
{
	template<>
	struct string_traits<net::http::message_tag>
		: string_traits<net::default_tag>
	{
	};
			
	template<>
	struct header_collection_traits<net::http::message_tag>
		: header_collection_traits<net::default_tag>
	{
	};	
	
	template<>
	struct char_traits< net::http::message_tag >
	: char_traits< net::default_tag >
	{		
	};
	
	namespace http
	{
		template<typename Tag>
		struct chunk_cache_traits{
			typedef std::list< 
					std::vector< typename char_traits< Tag >::type > 
				> type;					
		};
		
		template<typename Tag>
		struct parser_traits
		{
			enum{
				STATUS_MESSAGE_MAX 	= 1024u,
				HEADER_NAME_MAX 	= 1024u,
				HEADER_VALUE_MAX 	= 1024u,
				RESOURCE_MAX	 	= 1024u,
				QUERY_STRING_MAX 	= 1024u
			};
			
			typedef typename char_traits< Tag >::type char_type;
			
			// returns true if the argument is a special character
			inline static bool is_special( char_type c )
			{
				switch ( c )
				{
				case '(':
				case ')':
				case '<':
				case '>':
				case '@':
				case ',':
				case ';':
				case ':':
				case '\\':
				case '"':
				case '/':
				case '[':
				case ']':
				case '?':
				case '=':
				case '{':
				case '}':
				case ' ':
				case '\t':
					return true;
				default:
					return false;
				}
			}

			// returns true if the argument is a character
			inline static bool is_char( typename boost::mpl::if_< boost::is_signed<char_type>, boost::int32_t, boost::uint32_t>::type c )
			{
				return( c >= 0 && c <= 127 );
			}

			// returns true if the argument is a control character
			inline static bool is_control( char_type c )
			{
				return( ( c >= 0 && c <= 31 ) || c == 127 );
			}

			// returns true if the argument is a digit
			inline static bool is_digit( char_type c )
			{
				return( c >= '0' && c <= '9' );
			}

			// returns true if the argument is a hexadecimal digit
			inline static bool is_hex_digit( char_type c )
			{
				return( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) );
			}
			
		};
	}
}

#endif //GUARD_NET_HTTP_DETAIL_TRAITS_HPP_INCLUDED
