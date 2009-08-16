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
#ifndef GUARD_NET_CLIENT_PROXY_BASE_HPP_INCLUDED
#define GUARD_NET_CLIENT_PROXY_BASE_HPP_INCLUDED

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace net
{
    template<typename Tag>
    struct proxy_socket;
    
    template<typename Tag>
    struct proxy_base
    {        
        typedef boost::shared_ptr<proxy_base<Tag> >             self_ptr;
        typedef boost::asio::ip::tcp::resolver                  resolver;
        typedef boost::asio::ip::tcp::endpoint                  endpoint_type;
        typedef boost::asio::ip::tcp::resolver::iterator        endpoint_iterator;
        typedef boost::asio::io_service                         service_type;
        typedef boost::system::error_code                       error_code;
        typedef boost::function< void(error_code const & ec) >  connected_handler;
        typedef typename net::string_traits<Tag>::type          string_type;

        proxy_base(service_type & service)
            : resolver_(service)
        {}

        void set_server(string_type const & server, string_type const & port)
        {
            server_ = server;
            port_    = port;
        }

        virtual ~proxy_base()
        {}

        void cancel(error_code & ec)
        {
            ec = error_code();
            resolver_.cancel();
        }

#ifndef BOOST_NO_EXCEPTIONS
        void cancel()
        {
            error_code ec;
            cancel(ec);
            boost::asio::detail::throw_error(ec);
        }
#endif //#ifndef BOOST_NO_EXCEPTIONS

        virtual error_code connect(proxy_socket<Tag> & socket, endpoint_type const & endpoint, error_code & ec)
        {
            // Dummy implementations for the empty proxy
			socket.lowest_layer().close(); 
            return socket.lowest_layer().connect(endpoint, ec);
        }

        virtual void async_connect(
            proxy_socket<Tag> & socket, 
            endpoint_type const & endpoint,
            connected_handler connected
        )
        {
			if(!connected)
			{
				//We're not doing all the work 
				//Just to fail on calling the handler
				throw boost::system::system_error(boost::asio::error::invalid_argument);
			}

            // Dummy implementations for the empty proxy
			socket.lowest_layer().close(); 
            socket.lowest_layer().async_connect(
                endpoint,
                connected
            );
        }
    
    protected:
        virtual error_code internal_connect(
            proxy_socket<Tag> & socket, 
            endpoint_type const & endpoint, 
            error_code & ec
        )
        {
            resolver::query query(server_, port_);
            endpoint_iterator iter = resolver_.resolve(query, ec);
            while(iter != endpoint_iterator())
            {
                endpoint_type ep = *iter;
                if(!socket.lowest_layer().connect(ep, ec))
                {
                    return on_connected(socket, endpoint, ec);
                }
                ++iter;
            }
            return ec;
        }

        virtual error_code on_connected(
            proxy_socket<Tag> &, 
            endpoint_type const &, 
            error_code & ec)
        {
            return ec;    
        }

        virtual void internal_async_connect(
            proxy_socket<Tag> &        socket, 
            endpoint_type const &   endpoint,
            connected_handler        connected
        )
        {
            resolver::query query(server_, port_);
            resolver_.async_resolve(
                query,
                boost::bind(
                    &proxy_base::on_resolved,
                    this,
                    boost::asio::placeholders::iterator,
                    boost::asio::placeholders::error,
                    boost::ref(socket),
                    endpoint,
                    connected
                )
            );
        }

        virtual void do_async_connect(
            endpoint_iterator ep_iter,
            proxy_socket<Tag> &    socket, 
            endpoint_type const & endpoint,
            connected_handler connected
        )
        {
            endpoint_type ep = *ep_iter;
			std::cout << "Trying to connect to proxy: " << ep.address().to_string() << ":" << ep.port() << std::endl;
            socket.next_layer().async_connect(
            ep,
                boost::bind(
                    &proxy_base::on_async_connection_result,
                    this,
                    boost::asio::placeholders::error,
                    ++ep_iter,
                    boost::ref(socket),
                    endpoint,
                    connected
                )
            );
        }
        
        virtual void on_resolved(
            endpoint_iterator ep_iter,
            error_code const &    ec, 
            proxy_socket<Tag> &    socket, 
            endpoint_type const & endpoint,
            connected_handler connected
        )
        {
            if(!ec)
            {
                do_async_connect(ep_iter, socket, endpoint, connected);
            }
            else
            {
                connected(ec);
            }
        }

        virtual void on_async_connection_result(
            error_code const & ec, 
            endpoint_iterator epiter,
            proxy_socket<Tag> &    socket, 
            endpoint_type const & endpoint,
            connected_handler connected
        )
        {
            if(!ec)
            {
                on_async_connected(socket, endpoint, connected);
            }
            else if(ec && epiter != endpoint_iterator())
            {
                do_async_connect(epiter, socket, endpoint, connected);
            }
            else if(ec && epiter == endpoint_iterator())
            {
                connected(ec);
            }
        }

        virtual void on_async_connected(
            proxy_socket<Tag> &, 
            endpoint_type const &,
            connected_handler
        )
        {
            
        }
    protected:
        resolver    resolver_;
        string_type server_;
        string_type port_;
    };

    template<typename Tag>
    struct implements_proxy : proxy_base<Tag>
    {
        typedef proxy_base<Tag>                         base_type;
        typedef typename base_type::service_type        service_type;
        typedef typename base_type::endpoint_type       endpoint_type;
        typedef typename base_type::connected_handler   connected_handler;
        typedef typename base_type::error_code          error_code;

        implements_proxy(service_type & service)
            : base_type(service)
        {}            

        virtual error_code connect(proxy_socket<Tag> & socket, endpoint_type const & endpoint, error_code & ec)
        {
			// Ensure the socket is closed before we're going to do anything
			socket.lowest_layer().close(ec);
			if(!ec)
			{
				return this->internal_connect(socket, endpoint, ec);
			}
			return ec;
        }

        virtual void async_connect(
            proxy_socket<Tag> & socket, 
            endpoint_type const & endpoint,
            connected_handler connected
            )
        {
			if(!connected)
			{
				//We're not doing all the work 
				//Just to fail on calling the handler
				throw boost::system::system_error(boost::asio::error::invalid_argument);
			}

			socket.lowest_layer().close();
            this->internal_async_connect(socket, endpoint, connected);
        }
    };
}

#endif //GUARD_NET_CLIENT_PROXY_BASE_HPP_INCLUDED
