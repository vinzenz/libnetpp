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
#ifndef GUARD_NET_HTTP_COOKIE_HPP_INCLUDED
#define GUARD_NET_HTTP_COOKIE_HPP_INCLUDED

#include <net/http/detail/traits.hpp>

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
        template<typename Tag>
        class basic_cookie
        {
        public:
            typedef typename string_traits<Tag>::type string_type;
            typedef string_traits<Tag> string_traits_type;
            
        private:
            string_type name_;
            string_type value_;

            string_type comment_;
            string_type domain_;
            string_type max_age_;
            string_type path_;
            string_type version_;
            string_type expires_;
            bool http_only_;
            bool secure_;
        public:
            basic_cookie()
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

            basic_cookie( basic_cookie const & other )
                    : name_( other.name_ )
                    , value_( other.value_ )
                    , comment_( other.comment_ )
                    , domain_( other.domain_ )
                    , max_age_( other.max_age_ )
                    , path_( other.path_ )
                    , version_( other.version_ )
                    , expires_( other.expires_ )
                    , http_only_( other.http_only_ )
                    , secure_( other.secure_ )
            {
            }

            virtual ~basic_cookie()
            {

            }

            basic_cookie & operator=( basic_cookie other )
            {
                swap( other );
                return *this;
            }

            void swap( basic_cookie & other )
            {
                name_.swap( other.name_ );
                value_.swap( other.value_ );
                comment_.swap( other.comment_ );
                domain_.swap( other.domain_ );
                max_age_.swap( other.max_age_ );
                path_.swap( other.path_ );
                version_.swap( other.version_ );
                expires_.swap( other.expires_ );
                std::swap( http_only_,other.http_only_ );
                std::swap( secure_,other.secure_ );
            }

            string_type const & name() const
            {
                return name_;
            }

            string_type & name()
            {
                return name_;
            }

            string_type const & value() const
            {
                return value_;
            }

            string_type & value()
            {
                return value_;
            }

            string_type const & comment() const
            {
                return comment_;
            }

            string_type & comment()
            {
                return comment_;
            }

            string_type const & domain() const
            {
                return domain_;
            }

            string_type & domain()
            {
                return domain_;
            }

            string_type const & max_age() const
            {
                return max_age_;
            }

            string_type & max_age()
            {
                return max_age_;
            }

            string_type const & path() const
            {
                return path_;
            }

            string_type & path()
            {
                return path_;
            }

            string_type const & version() const
            {
                return version_;
            }

            string_type & version()
            {
                return version_;
            }

            string_type const & expires() const
            {
                return expires_;
            }

            string_type & expires()
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

            string_type as_string( string_type const & str ) const
            {
                if ( version() == string_traits_type::convert("0") )
                    return string_traits_type::convert("\"") + str + string_traits_type::convert("\"");
                return str;
            }

            string_type build() const
            {
                string_type res = name() + string_traits_type::convert("=") + as_string( value() );
                if ( !comment().empty() )
                    res += string_traits_type::convert("; comment=") + as_string( comment() );
                if ( !expires().empty() )
                    res += string_traits_type::convert("; expires=") + as_string( expires() );
                if ( !max_age().empty() )
                    res += string_traits_type::convert("; max-Age=") + as_string( max_age() );
                if ( !path().empty() )
                    res += string_traits_type::convert("; path=") + as_string( path() );
                if ( !domain().empty() )
                    res += string_traits_type::convert("; domain=") + as_string( domain() );
                if ( !version().empty() )
                    res += string_traits_type::convert("; version=") + as_string( version() );
                if ( http_only() )
                    res += string_traits_type::convert("; httponly");
                if ( secure() )
                    res += string_traits_type::convert("; secure");
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
        template<typename Tag>
        class basic_cookie2
                    : public basic_cookie<Tag>
        {
            typedef typename basic_cookie<Tag>::string_type string_type;
            typedef std::list<boost::uint16_t> port_list_t;
            string_type comment_url_;
            bool discard_;
            port_list_t ports_;
        public:
            basic_cookie2()
                    : basic_cookie<Tag>()
                    , comment_url_()
                    , discard_()
                    , ports_()
            {

            }

            basic_cookie2( basic_cookie2 const & other )
                    : basic_cookie<Tag>( other )
                    , comment_url_( other.comment_url_ )
                    , discard_( other.discard_ )
                    , ports_( other.ports_ )
            {

            }

            virtual ~basic_cookie2()
            {

            }

            basic_cookie2 & operator=( basic_cookie2 other )
            {
                swap( other );
                return *this;
            }

            void swap( basic_cookie2 & other )
            {
                basic_cookie<Tag>::swap( other );
                comment_url_.swap( other.comment_url_ );
                std::swap( discard_, other.discard_ );
                ports_.swap( other.ports_ );
            }

            string_type const & comment_url() const
            {
                return comment_url_;
            }

            string_type & comment_url()
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
