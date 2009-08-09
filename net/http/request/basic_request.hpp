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
#ifndef GUARD_NET_HTTP_REQUEST_BASIC_REQUEST_HPP_INCLUDED
#define GUARD_NET_HTTP_REQUEST_BASIC_REQUEST_HPP_INCLUDED

#include <net/http/basic_message.hpp>

namespace net
{
	namespace http
	{
		template<typename Tag>
		class basic_request : public http::basic_message<Tag>
		{
			typedef http::basic_message<Tag> base_type;
			typedef typename string_traits<Tag>::type string_type;
			string_type method_;
			string_type resource_;
			string_type query_;
		public:
			basic_request()
			: base_type()
			, method_()
			{
				
			}
			
			basic_request(basic_request const & other)
			: base_type(other)
			, method_(other.method_)
			{
				
			}
			
			basic_request & operator=(basic_request other)
			{
				swap(other);
				return *this;
			}
			
			string_type & method()
			{
				return method_;
			}
			
			string_type const & method() const
			{
				return method_;
			}					
			
			string_type & resource()
			{
				return resource_;
			}
			
			string_type const & resource() const
			{
				return resource_;
			}					
			
			string_type & query()
			{
				return query_;
			}
			
			string_type const & query() const
			{
				return query_;
			}					
						
			void swap(basic_request & other)
			{
				base_type & other_(other);
				base_type & this_(*this);
				other_.swap(this_);
			}
		};
	}
}

#endif //GUARD_NET_HTTP_REQUEST_BASIC_REQUEST_HPP_INCLUDED
