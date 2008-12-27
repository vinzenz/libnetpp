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
#ifndef GUARD_NET_HTTP_COOKIE_HPP_INCLUDED
#define GUARD_NET_HTTP_COOKIE_HPP_INCLUDED

namespace net
{
	namespace http
	{
		/**
		 * \class cookie
		 * \author Vinzenz Feenstra
		 * \date 23.12.2008
		 * \file cookie.hpp
		 * \brief RFC 2109 a-like values for cookies
		 */
		class cookie
		{	
			std::string name_;
			std::string value_;
			
			std::string comment_;
			std::string domain_;
			std::string max_age_;
			std::string path_;
			std::string version_;
			std::string expires_;
			bool http_only_;
			bool secure_;
		public:
			cookie()
			: name_()
			, value_()
			, comment_()
			, domain_()
			, max_age_()
			, path_()
			, version_()
			, expires_()
			, http_only_()
			, secure_()
			{				
			}
		
			cookie(cookie const & other)
			: name_(other.name_)
			, value_(other.value_)
			, comment_(other.comment_)
			, domain_(other.domain_)
			, max_age_(other.max_age_)
			, path_(other.path_)
			, version_(other.version_)
			, expires_(other.expires_)
			, http_only_(other.http_only_)
			, secure_(other.secure_)
			{				
			}

			virtual ~cookie()
			{
				
			}
		
			cookie & operator=(cookie other)
			{
				swap(other);
				return *this;
			}

			void swap(cookie & other)
			{
				name_.swap(other.name_);
				value_.swap(other.value_);
				comment_.swap(other.comment_);
				domain_.swap(other.domain_);
				max_age_.swap(other.max_age_);
				path_.swap(other.path_);
				version_.swap(other.version_);
				expires_.swap(other.expires_);
				std::swap(http_only_,other.http_only_);
				std::swap(secure_,other.secure_);
			}
		
			std::string const & name() const 
			{
				return name_;				
			}
			
			std::string & name() 
			{
				return name_;
			}
			
			std::string const & value() const 
			{
				return value_;				
			}
			
			std::string & value() 
			{
				return value_;
			}
			
			std::string const & comment() const 
			{
				return comment_;				
			}
			
			std::string & comment() 
			{
				return comment_;
			}
			
			std::string const & domain() const 
			{
				return domain_;				
			}
			
			std::string & domain() 
			{
				return domain_;
			}
			
			std::string const & max_age() const 
			{
				return max_age_;				
			}
			
			std::string & max_age() 
			{
				return max_age_;
			}
			
			std::string const & path() const 
			{
				return path_;				
			}
			
			std::string & path() 
			{
				return path_;
			}
			
			std::string const & version() const 
			{
				return version_;				
			}
			
			std::string & version() 
			{
				return version_;
			}
			
			std::string const & expires() const
			{
				return expires_;
			}
			
			std::string & expires()
			{
				return expires_;
			}
			
			bool http_only() const
			{
				return http_only_;
			}
			
			bool & http_only()
			{
				return http_only_;
			}
			
			bool secure() const 
			{
				return secure_;				
			}
			
			bool & secure() 
			{
				return secure_;
			}
            
            std::string as_string(std::string const & str) const
            {
                if(version() == "0")
                    return "\"" + str + "\"";
                return str;
            }
            
            std::string build() const
            {
                std::string res = name() + "=" + as_string(value());
                if(!comment().empty())
                    res += "; comment=" + as_string(comment());
                if(!expires().empty())
                    res += "; expires=" + as_string(expires());
                if(!max_age().empty())
                    res += "; max-Age=" + as_string(max_age());
                if(!path().empty())
                    res += "; path=" + as_string(path());
                if(!domain().empty())
                    res += "; domain=" + as_string(domain());
                if(!version().empty())
                    res += "; version=" + as_string(version());
                if(http_only())
                    res += "; httponly";
                if(secure())
                    res += "; secure";
                return res;
            }
		};
		
		/**
		 * \class cookie2
		 * \author Vinzenz Feenstra
		 * \date 23.12.2008
		 * \file cookie.hpp
		 * \brief RFC 2965 a-like extension to the cookie values
		 */
		class cookie2 
		: public cookie
		{
			typedef std::list<boost::uint16_t> port_list_t;
			std::string comment_url_;
			bool discard_;
			port_list_t ports_;
		public:
			cookie2()
			: cookie()
			, comment_url_()
			, discard_()
			, ports_()
			{
				
			}
			
			cookie2(cookie2 const & other)
			: cookie(other)
			, comment_url_(other.comment_url_)
			, discard_(other.discard_)
			, ports_(other.ports_)
			{
				
			}
			
			virtual ~cookie2()
			{
				
			}
			
			cookie2 & operator=(cookie2 other)
			{
				swap(other);
				return *this;
			}
			
			void swap(cookie2 & other)
			{
				cookie::swap(other);
				comment_url_.swap(other.comment_url_);
				std::swap(discard_, other.discard_);
				ports_.swap(other.ports_);
			}
			
			std::string const & comment_url() const
			{
				return comment_url_;
			}
			
			std::string & comment_url()
			{
				return comment_url_;
			}
			
			bool discard() const
			{
				return discard_;
			}
			
			bool & discard()
			{
				return discard_;
			}
			
			port_list_t const & ports() const
			{
				return ports_;
			}

			port_list_t & ports()
			{
				return ports_;
			}
		};			
	}
}

#endif //GUARD_NET_HTTP_COOKIE_HPP_INCLUDED
