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
#ifndef GUARD_NET_DETAIL_TRAITS_HPP_INCLUDED
#define GUARD_NET_DETAIL_TRAITS_HPP_INCLUDED

#include <net/detail/tags.hpp>
#include <string>
#include <list>
#include <vector>
#include <map>

namespace net
{
	template<typename Tag>
	struct string_traits;

	template<>
	struct string_traits<net::default_tag>
	{
		typedef std::string type;
        
        //TODO: Conversion needs to be implemented
        template<typename T>
        static type convert(T const * t)
        {
            return t;
        }
        
        template<typename T>
        static type convert(T const & t)
        {
            type str;
            str += t;
            return str;            
        }        
	};

	template<typename Tag>
	struct char_traits;
	
	template<>
	struct char_traits< net::default_tag >
	{
		typedef string_traits< net::default_tag >::type::value_type type;
	};


	template<typename Tag>
	struct header_collection_traits;

	template<>
	struct header_collection_traits<net::default_tag>
	{
		typedef std::multimap< string_traits<net::default_tag>::type,
							   string_traits<net::default_tag>::type > type;
	};
}

#endif //GUARD_NET_DETAIL_TRAITS_HPP_INCLUDED
