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
#ifndef GUARD_NET_BASIC_MESSAGE_HPP_INCLUDED
#define GUARD_NET_BASIC_MESSAGE_HPP_INCLUDED

#include <net/detail/traits.hpp>

namespace net
{
	template<typename Tag>
	class basic_message
	{
		typedef typename header_collection_traits<Tag>::type headers_type;
		typedef typename string_traits<Tag>::type string_type;
		
		headers_type headers_;
		string_type body_;
		string_type source_;
		string_type target_;
	public:
		
		basic_message()
		: headers_()
		, body_()
		, source_()
		, target_()
		{
			
		}
		
		basic_message(basic_message const & other)
		: headers_(other.headers_)
		, body_(other.body_)
		, source_(other.source_)
		, target_(other.target_)
		{
			
		}

		~basic_message()
		{			
		}

		basic_message & operator=(basic_message other)
		{
			swap(*this);
			return *this;
		}
		
		void swap(basic_message & other)
		{
			std::swap(other.headers_, headers_);
			std::swap(other.body_, body_);
			std::swap(other.source_, source_);
			std::swap(other.target_, target_);
		}
		
		headers_type const & headers() const
		{
			return headers_;
		}
		
		headers_type & headers()
		{
			return headers_;			
		}
		
		string_type & body()
		{
			return body_;
		}

		string_type const & body() const
		{
			return body_;
		}
		
		string_type const & source() const
		{
			return source_;
		}

		string_type & target()
		{
			return target_;
		}

		string_type const & target() const
		{
			return target_;
		}

	};	
}

#endif //GUARD_NET_HTTP_BASIC_MESSAGE_HPP_INCLUDED
