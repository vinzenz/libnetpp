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
#ifndef GUARD_NET_HTTP_basic_message_HPP_INCLUDED
#define GUARD_NET_HTTP_basic_message_HPP_INCLUDED

#include <boost/cstdint.hpp>
#include <net/http/detail/traits.hpp>
#include <net/basic_message.hpp>

namespace net
{
	namespace http
	{
		template<typename Tag>
		class basic_message : public net::basic_message<Tag>
		{
			typedef net::basic_message<Tag> base_type;
			typedef typename string_traits<Tag>::type string_type;
			typedef std::pair<boost::uint8_t, boost::uint8_t> version_type;
			typedef boost::uint16_t status_code_type;

			boost::uint16_t status_code_;
			version_type version_;
			string_type status_msg_;
		public:

			basic_message()
			: base_type()
			, status_code_(0)
			, version_(1,0)
			, status_msg_()
			{			
			}
			
			basic_message(basic_message const & other)
			: base_type(other)
			, status_code_(other.status_code_)
			, version_(other.version_)
			, status_msg_(other.status_msg_)
			{		
			}

			virtual ~basic_message()
			{
			}

			basic_message & operator=(basic_message other)
			{
				swap(other);
				return *this;
			}

			status_code_type & status_code()
			{
				return status_code_;
			}
			
			status_code_type const & status_code() const
			{
				return status_code_;
			}

			string_type & status_message()
			{
				return status_msg_;
			}
			
			string_type const & status_message() const
			{
				return status_msg_;
			}
			
			version_type & version()
			{
				return version_;
			}
			
			version_type const & version() const
			{
				return version_;
			}
			
			void swap(basic_message & other)
			{
				base_type & this_(*this);
				this_.swap(other);
				std::swap(other.version_, version_);
				std::swap(other.status_code_, status_code_);
				std::swap(other.status_msg_, status_msg_);
			}			
		};
	}
}

#endif //GUARD_NET_HTTP_basic_message_HPP_INCLUDED
