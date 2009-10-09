#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

template<typename DumpPolicy>
struct server
{
	typedef DumpPolicy dump_policy;

	typedef boost::asio::ip::tcp		protocol_type;
	typedef protocol_type::endpoint		endpoint_type;
	typedef boost::asio::io_service		service_type;
	typedef boost::system::error_code	error_code;
	
	struct session;
	typedef boost::shared_ptr<session> session_ptr;

	struct session
		: boost::enable_shared_from_this<session>
	{
		session(service_type & service, boost::uint32_t id)
			: id_(id)
			, service_(service)
			, socket_(service_)
			, buffer_()
			, index_(0)
		{
		}

		~session()
		{
			dump_policy::text(id_, "~session", error_code());
			socket_.close();
		}

		void start()
		{
			boost::asio::async_read(
				socket_, 
				boost::asio::buffer(buffer_), 
				boost::asio::transfer_at_least(1), 
				boost::bind(&session::on_data_read, this, _1, _2, this->shared_from_this())
			);
		}

		void on_data_read(error_code const & ec, size_t bytes_read, session_ptr)
		{
			dump_policy::dump(id_, buffer_, bytes_read, ec);
			if(!ec)
			{
				boost::array<boost::uint8_t, 10> buffer;
				buffer[0] = 0x05;
				buffer[1] = 0x00;
				buffer[2] = 0x00;
				buffer[4] = 0x01;
				switch(index_++)
				{
				case 0:
					boost::asio::write(socket_, boost::asio::buffer(buffer.data(), 2));
					break;
				case 1:
					boost::asio::write(socket_, boost::asio::buffer(buffer.data(), 10));
					break;
				default:
					break;
				}
				start();
			}
		}

		protocol_type::socket & socket(){ return socket_; }

	protected:
		boost::uint32_t id_;
		service_type & service_;
		protocol_type::socket socket_;
		boost::array<boost::uint8_t, 0x10000> buffer_;
		int index_;
	};

	server(service_type & service, boost::uint16_t port)
		: port_(port)
		, service_(service)
		, acceptor_(service)
		, endpoint_(endpoint_type(protocol_type::v4(), port))
		, session_()
//		, work_(new service_type::work(service))
		, next_id_(0)
	{		
		acceptor_.open(endpoint_.protocol());
		acceptor_.bind(endpoint_);
		acceptor_.listen();
		accept_next();
	}

	~server()
	{
//		work_.reset();
	}

	void accept_next()
	{
		session_.reset( new session(service_, ++next_id_) );
		acceptor_.async_accept(
			session_->socket(),
			boost::bind( &server<DumpPolicy>::on_accepted, this, _1 )
		);
	}

	void on_accepted(error_code const & ec)
	{
		if(!ec)
		{
			session_->start();
			accept_next();
		}
		else
		{
			session_.reset();
		}
	}

	boost::uint16_t port_;
	boost::asio::io_service & service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	endpoint_type endpoint_;
	session_ptr session_;
	boost::uint32_t next_id_;
//	boost::shared_ptr<service_type::work> work_;
};

struct console_dump
{
	template<typename IdType, typename ArrayT, typename error_code>
	static void text(IdType id, ArrayT const & p1, error_code const & ec)
	{
		if(ec)
		{
			std::cout << "[" << id << "] Failure: " << ec << " Message: " << ec.message() << "\n";
		}
		if(p1)
		{
			std::cout << "[" << id << "] Data received:\n" << p1 << "\n";
		}
		std::cout << "\n\n";
	}

	template<typename IdType, typename ArrayT, typename error_code>
	static void dump(IdType id, ArrayT const & p1, size_t bytes_read, error_code const & ec)
	{
		if(ec)
		{
			std::cout << "[" << id << "] Failure: " << ec << " Message: " << ec.message() << "\n";
		}
		if(bytes_read)
		{
			std::cout << "[" << id << "] Data received:\n";
			static char const hex_chars[17] = "0123456789ABCDEF";
			for(size_t i = 0; i < bytes_read; ++i)
			{
				boost::uint8_t b = p1[i];
				
				std::cout << " " << hex_chars[ (b & 0xF0) >> 4 ] << hex_chars[ b & 0x0F ];
			}
		}
		std::cout << "\n\n";
	}
};

int main()
{
	boost::asio::io_service service;
	server<console_dump> serve(service, 9080);	
	service.run();
	
	return 0;
}
