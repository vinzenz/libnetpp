/*
 * Copyright (c) 2008, Vinzenz Feenstra
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
#ifndef GUARD_NET_HTTP_COOKIE_JAR_HPP_INCLUDED
#define GUARD_NET_HTTP_COOKIE_JAR_HPP_INCLUDED
 
#include <net/http/detail/traits.hpp>
#include <boost/shared_ptr.hpp>
#include <net/http/cookie.hpp>
#include <map>
 
 namespace net
 {
     namespace http
     {
                 template<typename Tag>
        class basic_cookie_jar
        {
            typedef typename string_traits<Tag>::type string_type;
            typedef std::multimap< string_type , boost::shared_ptr<basic_cookie<Tag> > > jar_type;
            jar_type jar_;
        public:
            basic_cookie_jar()
            {

            }

            basic_cookie_jar( basic_cookie_jar const & other )
            {

            }

            ~basic_cookie_jar()
            {

            }

            basic_cookie_jar & operator=( basic_cookie_jar other )
            {
                swap( other );
                return *this;
            }

            void swap( basic_cookie_jar & )
            {

            }

            void add(string_type const & domain, boost::shared_ptr<basic_cookie<Tag> > c)
            {
                if(jar_.count(domain))
                {
                    typedef typename jar_type::iterator iterator_t;
                    typedef std::pair<iterator_t, iterator_t> iterator_range_t;
                    iterator_range_t range = jar_.equal_range(domain);
                    for(;range.first != range.second; ++range.first)
                    {
                        if(range.first->second->name() == c->name())
                        {
                            range.first->second = c;
                            return;
                        }
                    }
                }
                jar_.insert(std::make_pair(domain, c));
            }
        };
     }
 }
 
 #endif // GUARD_NET_HTTP_COOKIE_JAR_HPP_INCLUDED
 
