#ifndef GUARD_NET_CLIENT_SOCKET_ADAPTER_HPP_INCLUDED
#define GUARD_NET_CLIENT_SOCKET_ADAPTER_HPP_INCLUDED

#include <net/client/connection.hpp>

namespace net
{
    template<typename Tag>
    struct socket_adapter
    {           
        typedef boost::shared_ptr< connection_base<Tag> >   connection_ptr;
        typedef boost::asio::socket_base                    socket_base;
        typedef typename connection_base<Tag>::service_type service_type;

        typedef connection<Tag>                             connection_type;
        typedef ssl_connection<Tag>                         ssl_connection_type;

        typedef typename ssl_connection_type::socket_type   ssl_socket_type;
        typedef typename connection_type::socket_type       socket_type;

        socket_adapter(connection_ptr connection, bool ssl)
        : connection_(connection)
        , ssl_(ssl)
        {}

        socket_adapter(socket_adapter const & sa)
        : connection_(sa.connection_)
        , ssl_(sa.ssl_)
        {}
					
		~socket_adapter()
		{}
		
        socket_adapter & operator=(socket_adapter sa)
		{
			connection_.swap(sa.connection_);
			std::swap(ssl_, sa.ssl_);
			return *this;
		}

		service_type & get_io_service()
        {
            return ssl_ ? get_ssl_connection().socket().io_service()
                        : get_connection().socket().io_service();
        }              

         
        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence& buffers)
        {
            return ssl_ ? ssl_socket().send(buffers) 
                        : socket().send(buffers);
        }

        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence& buffers, socket_base::message_flags flags)
        {
            return ssl_ ? ssl_socket().send(buffers, flags)
                        : socket().send(buffers, flags);
        }
    
        template <typename ConstBufferSequence>
        std::size_t send(const ConstBufferSequence& buffers, socket_base::message_flags flags, boost::system::error_code& ec)
        {
            return ssl_ ? ssl_socket().send(buffers, flags, ec)
                        : socket().send(buffers, flags, ec);    
        }

        template <typename ConstBufferSequence, typename WriteHandler>
        void async_send(const ConstBufferSequence& buffers, WriteHandler handler)
        {
            ssl_ ? ssl_socket().async_send(buffers, handler)
                 : socket().async_send(buffers, handler);
        }

        template <typename ConstBufferSequence, typename WriteHandler>
        void async_send(const ConstBufferSequence& buffers, socket_base::message_flags flags, WriteHandler handler)
        {
            ssl_ ? ssl_socket().async_send(buffers, flags, handler)
                 : socket().async_send(buffers, flags, handler);
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence& buffers)
        {
            return ssl_ ? ssl_socket().receive(buffers)
                        : socket().receive(buffers);
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence& buffers,socket_base::message_flags flags)
        {
            return ssl_ ? ssl_socket().receive(buffers, flags)
                        : socket().receive(buffers, flags);
        }

        template <typename MutableBufferSequence>
        std::size_t receive(const MutableBufferSequence& buffers, socket_base::message_flags flags, boost::system::error_code& ec)
        {
            return ssl_ ? ssl_socket().receive(buffers, flags, ec)
                        : socket().receive(buffers, flags, ec);
        }

        template <typename MutableBufferSequence, typename ReadHandler>
        void async_receive(const MutableBufferSequence& buffers, socket_base::message_flags flags, ReadHandler handler)
        {
            return ssl_ ? ssl_socket().async_receive(buffers, flags, handler)
                        : socket().async_receive(buffers, flags, handler);
        }


        template <typename MutableBufferSequence, typename ReadHandler>
        void async_receive(const MutableBufferSequence& buffers, ReadHandler handler)
        {
            return ssl_ ? ssl_socket().async_receive(buffers, handler)
                        : socket().async_receive(buffers, handler);
        }


        template <typename ConstBufferSequence, typename WriteHandler>
        void async_write_some(const ConstBufferSequence& buffers, WriteHandler handler)
        {
            return ssl_ ? ssl_socket().async_write_some(buffers, handler)
                        : socket().async_write_some(buffers, handler);
        }


        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& buffers)
        {
            return ssl_ ? ssl_socket().write_some(buffers)
                        : socket().write_some(buffers);
        }

        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& buffers, boost::system::error_code& ec)
        {
            return ssl_ ? ssl_socket().write_some(buffers, ec)
                        : socket().write_some(buffers, ec);
        }


        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence& buffers)
        {
            return ssl_ ? ssl_socket().read_some(buffers)
                        : socket().read_some(buffers);
        }


        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence& buffers, boost::system::error_code& ec)
        {
            return ssl_ ? ssl_socket().read_some(buffers, ec)
                        : socket().read_some(buffers, ec);
        }

        template <typename MutableBufferSequence, typename ReadHandler>
        void async_read_some(const MutableBufferSequence& buffers, ReadHandler handler)
        {
            return ssl_ ? ssl_socket().async_read_some(buffers, handler)
                        : socket().async_read_some(buffers, handler);
        }

		void set_proxy(typename proxy_base<Tag>::self_ptr ptr)
		{
			ssl_ ? ssl_socket().next_layer().set_proxy(ptr)
				:  socket().set_proxy(ptr);
		}

        socket_type & socket()
        {
            return get_connection().socket();
        }

        ssl_socket_type & ssl_socket()
        {
            return get_ssl_connection().socket();
        }

        connection_base<Tag> & base()
        {
            return *connection_;
        }

        connection_type & get_connection()
        { 
            return static_cast<connection_type&>(base()); 
        }

        ssl_connection_type & get_ssl_connection()
        { 
            return static_cast<ssl_connection_type&>(base()); 
        }

    protected:
        connection_ptr connection_;
        bool ssl_;

    };
}
#endif //GUARD_NET_CLIENT_SOCKET_ADAPTER_HPP_INCLUDED

