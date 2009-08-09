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
#ifndef GUARD_NET_HTTP_PARSER_COOKIE_PARSER_HPP_INCLUDED
#define GUARD_NET_HTTP_PARSER_COOKIE_PARSER_HPP_INCLUDED

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <net/http/cookie_jar.hpp>
#include <boost/noncopyable.hpp>

namespace net
{
    namespace http
    {
        template<typename Tag>
        class basic_cookie_parser 
            : boost::noncopyable
        {
            typedef typename string_traits<Tag>::type string_type;
            typedef string_traits<Tag> string_traits_type;
        public:
            basic_cookie_parser()
            : boost::noncopyable()
            {

            }

            bool parse( basic_cookie_jar<Tag> & jar, basic_message<Tag> & message )
            {
                typedef typename header_collection_traits<Tag>::type::const_iterator iterator_t;
                typedef std::pair<iterator_t, iterator_t> iterator_range_t;

                iterator_range_t range = message.headers().equal_range( string_traits_type::convert("Set-Cookie") );
                for ( iterator_t iter = range.first; iter != range.second; ++iter )
                {
                    do_parse( jar, message.target(), iter->second );                    
                }
                return true;
            }

        private:            
            void do_parse( basic_cookie_jar<Tag> & jar, std::string const & domain, string_type const & data )
            {
                boost::shared_ptr<basic_cookie<Tag> > c( new basic_cookie<Tag>() );
                std::vector<string_type> parts;
                boost::split( parts, data, boost::is_any_of( string_traits_type::convert(";") ) );

                string_type name;
                string_type value;

                BOOST_FOREACH( string_type const & str, parts )
                {
                    if ( name.empty() )
                    {
                        string_type tmp = boost::trim_copy( str );
                        if ( !tmp.empty() )
                        {
                            extract( tmp, name, value, *c );
                        }
                    }
                    else
                    {
                        value += string_traits_type::convert(";") + str;
                        if ( *str.rbegin() == '"' )
                        {
                            extract( value, name, value,*c );
                        }
                    }                                    
                }
                
                jar.add(domain, c);
            }

            void extract( string_type const & data, string_type & name, string_type & value, basic_cookie<Tag> & c )
            {
                std::size_t pos = data.find( string_traits_type::convert('=') );
                name = data.substr( 0,pos );
                if ( pos != string_type::npos )
                {
                    value = data.substr( pos+1 );
                }

                if ( !value.empty() )
                {
                    if ( *value.begin() == '"' && *value.rbegin() != '"' )
                    {
                        value = name + string_traits_type::convert("=") + value;
                        return;
                    }
                    boost::trim_if( value, boost::is_any_of( string_traits_type::convert("\"") ) );
                }
                set_value( name, value, c );
                name.clear();
                value.clear();
            }

            void set_value( string_type const & name, string_type const & value, basic_cookie<Tag> & c )
            {
                string_type lname = boost::to_lower_copy(name);
                
                if ( lname == string_traits_type::convert("max-age") )
                {
                    c.max_age() = value;
                }
                else if ( lname == string_traits_type::convert("domain") )
                {
                    c.domain() = value;
                }
                else if ( lname == string_traits_type::convert("comment") ) 
                {
                    c.comment() = value;
                }
                else if ( lname == string_traits_type::convert("path") )
                {
                    c.path() = value;
                }
                else if ( lname == string_traits_type::convert("version") )
                {
                    c.version() = value;
                }
                else if ( lname == string_traits_type::convert("expires") )
                {
                    c.expires() = value;
                }
                else if ( lname == string_traits_type::convert("httponly") )
                {
                    c.http_only() = true;
                }
                else if ( lname == string_traits_type::convert("secure") )
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
