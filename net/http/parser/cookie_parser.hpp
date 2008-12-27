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

#include <net/http/detail/traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <net/http/cookie.hpp>
#include <map>

namespace net
{
    namespace http
    {
        class cookie_jar
        {
            std::multimap< std::string , boost::shared_ptr<cookie> > jar_;
        public:
            cookie_jar()
            {

            }

            cookie_jar( cookie_jar const & other )
            {

            }

            ~cookie_jar()
            {

            }

            cookie_jar & operator=( cookie_jar other )
            {
                swap( other );
                return *this;
            }

            void swap( cookie_jar & )
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

            bool parse( cookie_jar & jar, basic_message<Tag> & message )
            {
                typedef typename header_collection_traits<Tag>::type::const_iterator iterator_t;
                typedef std::pair<iterator_t, iterator_t> iterator_range_t;

                iterator_range_t range = message.headers().equal_range( "Set-Cookie" );
                for ( iterator_t iter = range.first; iter != range.second; ++iter )
                {
                    do_parse( jar, iter->second );                    
                }
                return true;
            }

        private:

            // NAME=VALUE; max-age=0; httpOnly; secure; discard; domain=someDomain;
            void do_parse( cookie_jar & jar, std::string const & data )
            {
                boost::shared_ptr<cookie> c( new cookie() );
                std::vector<std::string> parts;
                boost::split( parts, data, boost::is_any_of( ";" ) );
                std::cout << "Parsed Cookie: " << std::endl;

                std::string name;
                std::string value;

                BOOST_FOREACH( std::string const & str, parts )
                {
                    if ( name.empty() )
                    {
                        std::string tmp = boost::trim_copy( str );
                        if ( !tmp.empty() )
                        {
                            extract( tmp, name, value, *c );
                        }
                    }
                    else
                    {
                        value += ";" + str;
                        if ( *str.rbegin() == '"' )
                        {
                            extract( value, name, value,*c );
                        }
                    }                                    
                }
                
                std::cout << "Cookie: " << c->build() << std::endl;
            }

            void extract( std::string const & data, std::string & name, std::string & value, cookie & c )
            {
                std::size_t pos = data.find( '=' );
                name = data.substr( 0,pos );
                if ( pos != std::string::npos )
                {
                    value = data.substr( pos+1 );
                }

                if ( !value.empty() )
                {
                    if ( *value.begin() == '"' && *value.rbegin() != '"' )
                    {
                        value = name + "=" + value;
                        return;
                    }
                    boost::trim_if( value,boost::is_any_of( "\"" ) );
                }
                set_value( name, value, c );
                name.clear();
                value.clear();
            }

            void set_value( std::string const & name, std::string const & value, cookie & c )
            {
                std::string lname = boost::to_lower_copy(name);
                
                if ( lname == "max-age" )
                {
                    c.max_age() = value;
                }
                else if ( lname == "domain" )
                {
                    c.domain() = value;
                }
                else if ( lname == "comment" ) 
                {
                    c.comment() = value;
                }
                else if ( lname == "path" )
                {
                    c.path() = value;
                }
                else if ( lname == "version" )
                {
                    c.version() = value;
                }
                else if ( lname == "expires" )
                {
                    c.expires() = value;
                }
                else if ( lname == "httponly" )
                {
                    c.http_only() = true;
                }
                else if ( lname == "secure" )
                {
                    c.secure() = true;
                }
                else
                {
                    c.name() = name;
                    c.value() = value;
                }
            }
        };
    }
}

#endif //GUARD_NET_HTTP_PARSER_COOKIE_PARSER_HPP_INCLUDED
