/*
 * Copyright (c) 2008,2009 by Vinzenz Feenstra
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
#ifndef GUARD_NET_CLIENT_CONNECTION_HPP_INCLUDED
#define GUARD_NET_CLIENT_CONNECTION_HPP_INCLUDED

#include <net/detail/traits.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <net/client/proxy_socket.hpp>

namespace net
{
    template<typename Tag>
    struct connection_base
    {
        typedef boost::function< void(boost::system::error_code const &) > callback;
		typedef net::proxy_socket<Tag>					socket;
        typedef boost::asio::ip::tcp::resolver			resolver;
        typedef boost::asio::ip::tcp::endpoint			endpoint;
        typedef boost::asio::io_service					service_type;
        typedef boost::asio::ssl::context				ssl_context_type;
        typedef typename string_traits<Tag>::type		string_type;

        connection_base( service_type & service )
        : service_(service)
        , resolver_(service)
        , timer_(service)
		, connect_timeout_(boost::posix_time::seconds(30))
        {
        }

        virtual ~connection_base()
		{}


		virtual void set_connect_timeout(boost::posix_time::time_duration const & duration)
		{
			connect_timeout_ = duration;
		}

        virtual void async_connect(string_type const & server, string_type const & port, callback cb)
        {
            resolver::query query(server, port);
			resolver_.async_resolve(
				query,
				boost::bind(
					&connection_base::async_connect_timeout,
					this,
					boost::asio::placeholders::iterator,
					boost::asio::placeholders::error,
					cb
				)
			);
        }

		virtual boost::system::error_code connect(string_type const & server, string_type const & port, boost::system::error_code & ec)
		{
			resolver::query query(server, port);
			return connect(resolver_.resolve(query, ec), ec);
		}

		virtual socket & get_plain_socket() = 0;
    protected:
        virtual void async_connect_timeout(resolver::iterator epiter, boost::system::error_code const & ec, callback cb)
        {
			if(!ec)
			{
				async_connect(epiter, cb);
				if(connect_timeout_.total_milliseconds())
				{
					timer_.expires_from_now(connect_timeout_);
					timer_.async_wait(
						boost::bind( 
							&connection_base<Tag>::connect_timeout, 
							this, 
							boost::asio::placeholders::error
						)
					);
				}
			}
			else
			{
				if(ec ==  boost::asio::error::operation_aborted)
				{
					cb(boost::asio::error::timed_out);                
				}
				else
				{
					cb(ec);
				}
			}
        }

        virtual socket & get_next_layer() = 0;

        virtual void handle_connect( boost::system::error_code const & ec, resolver::iterator epiter, callback cb)
        {
            if(ec ==  boost::asio::error::operation_aborted)
            {
                cb(boost::asio::error::timed_out);                
            }
            else if(!ec || (ec && epiter == resolver::iterator()))
            {
				timer_.cancel();
                cb(ec);                    
            }
            else if(epiter != resolver::iterator())
            {
                async_connect_timeout(epiter, boost::system::error_code(), cb);
            }
        }

		virtual boost::system::error_code connect(typename resolver::iterator epiter, boost::system::error_code & ec)
		{
			if(!ec)
			{
				while(epiter != typename resolver::iterator())
				{
					endpoint ep = *epiter;
					get_next_layer().close();
					if(!get_next_layer().connect(ep, ec))
					{
						return ec;
					}
					++epiter;
				}
			}
			return ec;
		}
    protected:        
        virtual void async_connect(typename resolver::iterator epiter, callback cb)
        {
            endpoint ep = *epiter;
			get_next_layer().close();
            get_next_layer().async_connect
            (
                ep, 
                boost::bind
                ( 
                    &connection_base<Tag>::handle_connect,
                    this, 
                    boost::asio::placeholders::error, 
                    ++epiter, 
                    cb
                ) 
            );
        }

        virtual void connect_timeout( boost::system::error_code const & ec )
        {
            if(!ec)
            {
                get_next_layer().cancel();
            }
        }

    protected:
        service_type & service_;
        resolver resolver_;
        boost::asio::deadline_timer timer_;
		boost::posix_time::time_duration connect_timeout_;
    };

    template <typename Tag>
    struct ssl_connection : connection_base<Tag>
    {
        typedef connection_base<Tag>								 base_type;
        typedef typename base_type::service_type					 service_type;
        typedef typename base_type::resolver						 resolver;
        typedef typename base_type::callback						 callback;
        typedef typename base_type::ssl_context_type				 ssl_context_type; 
        typedef boost::asio::ssl::stream<typename base_type::socket> socket_type;

        ssl_connection(service_type & service, ssl_context_type & context)
        : base_type(service)
        , socket_(service, context)
        {}        

        socket_type & socket(){ return socket_; }

		typename base_type::socket & get_plain_socket(){ return socket_.next_layer(); }
    protected:
        typename socket_type::next_layer_type & 
        get_next_layer()
        {
            return socket_.next_layer();
        }

		virtual void handle_connect( boost::system::error_code const & ec, typename resolver::iterator epiter, callback cb)
        {
            if(ec ==  boost::asio::error::operation_aborted)
            {
                cb(boost::asio::error::timed_out);                
            }
            else if(ec && epiter == typename resolver::iterator())
            {
                cb(ec);                    
            }
            else if(!ec)
            {
                socket_.async_handshake(
                    boost::asio::ssl::stream_base::client,
                    boost::bind(
                        &ssl_connection::handle_handshake,
                        this,
                        boost::asio::placeholders::error,   
                        cb
                    )
                );
            }
            else if(epiter != typename resolver::iterator())
            {
                async_connect_timeout(epiter, boost::system::error_code(), cb);
            }
        }

		virtual boost::system::error_code connect(typename resolver::iterator epiter, boost::system::error_code & ec)
		{			
			if(!base_type::connect(epiter, ec))
			{
				return socket_.handshake(boost::asio::ssl::stream_base::client, ec);
			}
			return ec;
		}
		
		virtual void handle_handshake( boost::system::error_code const & ec, callback cb)
        {
            cb(ec);
        }

    protected:
        socket_type socket_;
    };

    template <typename Tag>
    struct connection : connection_base<Tag>
    {
        typedef connection_base<Tag>				base_type;
        typedef typename base_type::socket			socket_type;
        typedef typename base_type::service_type	service_type;

        connection(service_type & service)
        : base_type(service)
        , socket_(service)
        {}        

        socket_type & socket(){ return socket_; }
		typename base_type::socket & get_plain_socket(){ return socket_; }
    protected:
        socket_type & 
        get_next_layer()
        {
            return socket_;
        }

    protected:
        socket_type socket_;
    };
}

#endif //GUARD_NET_CLIENT_CONNECTION_HPP_INCLUDED

