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
#ifndef GUARD_NET_CLIENT_UTILS_BUFFER_HPP_INCLUDED
#define GUARD_NET_CLIENT_UTILS_BUFFER_HPP_INCLUDED

#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/detail/endian.hpp>
#include <boost/static_assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/array.hpp>
#include <vector>
#include <stdexcept>

namespace net
{
	namespace util
	{
		struct buffer_stream_base
		{
			virtual ~buffer_stream_base()
			{}
		};
		template<typename OctetIterator>
		struct unchecked_buffer_stream_base 
			: buffer_stream_base
		{
			typedef OctetIterator iterator;

			unchecked_buffer_stream_base()
			: buffer_stream_base()
			, begin_(0)
			, end_(0)
			, pos_(0)
			{
				BOOST_STATIC_ASSERT( sizeof(*begin_) == 1 );
			}

			unchecked_buffer_stream_base(iterator begin, iterator end)
			: buffer_stream_base()
			, begin_(begin)
			, end_(end)
			, pos_(begin)
			{
				BOOST_STATIC_ASSERT( sizeof(*begin_) == 1 );
			}

			template<typename T>
			unchecked_buffer_stream_base & write(T const & t)
			{
				BOOST_STATIC_ASSERT( boost::is_pod<T>::value == true );
				boost::uint8_t const * p = reinterpret_cast<boost::uint8_t const*>(&t);
				if(boost::is_integral<T>::value)
				{
#ifdef BOOST_LITTLE_ENDIAN
					pos_ = std::reverse_copy(p, p + sizeof(t), pos_);
#elif defined(BOOST_BIG_ENDIAN)				
					pos_ = std::copy(p, p + sizeof(t), pos_);
#else
#	error Platform not supported
#endif
				}
				else
				{
					pos_ = std::copy(p, p + sizeof(t), pos_);
				}
				return *this;
			}

			template<typename T>
			unchecked_buffer_stream_base & read(T & t)
			{
				t = read<T>();
				return *this;
			}

			template<typename T>
			T read()
			{
				BOOST_STATIC_ASSERT( boost::is_pod<T>::value == true );
				union conv
				{
					T value;
					boost::array<boost::uint8_t, sizeof(T)> bytes;
				};
				conv c;
				
				boost::uint8_t * p = c.bytes.data();
				if(boost::is_integral<T>::value)
				{
#ifdef BOOST_LITTLE_ENDIAN
					std::reverse_copy(pos_, pos_ + sizeof(T), p);
#elif defined(BOOST_BIG_ENDIAN)				
					std::copy(pos_, pos_ + sizeof(T), p);
#else
#	error Platform not supported
#endif
					pos_ += sizeof(T);
				}
				else
				{
					std::copy(pos_, pos_ + sizeof(T), p);
					pos_ += sizeof(T);
				}
				return c.value;
			}

		protected:
			void reset(iterator begin, iterator end)
			{
				begin_ = pos_ = begin;
				end_ = end;
			}

		protected:
			iterator begin_, end_, pos_;
		};

		template<typename OctetIterator>
		struct checked_buffer_stream_base
			: unchecked_buffer_stream_base<OctetIterator>
		{
			typedef unchecked_buffer_stream_base<OctetIterator> base_type;
			typedef typename base_type::iterator iterator;

			checked_buffer_stream_base()
				: base_type()
			{}

			checked_buffer_stream_base(iterator begin, iterator end)
				: base_type(begin, end)
			{}

			template<typename T>
			checked_buffer_stream_base & write(T const & t)
			{
				buffer_check<T>();
				base_type::template write<T>(t);
				return *this;
			}

			template<typename T>
			checked_buffer_stream_base & read(T & t)
			{

				return *this;
			}

			template<typename T>
			T read()
			{
				buffer_check<T>();
				return base_type::template read<T>();
			}

		protected:
			template<typename T>
			void buffer_check()
			{
				if( !(this->begin_ < this->end_)
				|| (this->pos_ + 4 > this->end_))
				{
					throw std::out_of_range("Trying to access buffers outside their range");
				}

			}
		};	

		template<typename BaseType>
		struct buffer_stream 
			: BaseType
		{
			typedef BaseType base_type;
			typedef typename base_type::iterator iterator;

			buffer_stream()
				: base_type()
			{}

			buffer_stream(iterator begin, iterator end)
				: base_type(begin, end)
			{}

			buffer_stream & read8(boost::int8_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & readu8(boost::uint8_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & read16(boost::int16_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & readu16(boost::uint16_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & read32(boost::int32_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & readu32(boost::uint32_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & read64(boost::int64_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & readu64(boost::uint64_t & v)
			{
				base_type::read(v);
				return *this;
			}

			buffer_stream & write8(boost::int8_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & writeu8(boost::uint8_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & write16(boost::int16_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & writeu16(boost::uint16_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & write32(boost::int32_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & writeu32(boost::uint32_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & write64(boost::int64_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & writeu64(boost::uint64_t v)
			{
				base_type::write(v);
				return *this;
			}

			buffer_stream & write(boost::asio::ip::address const & address)
			{
				if(address.is_v4())
				{
					base_type::write(address.to_v4().to_bytes());
				}
				else
				{
					base_type::write(address.to_v6().to_bytes());
				}
				return *this;
			}
		};

		struct unchecked_buffer_stream_adapter
			: buffer_stream< unchecked_buffer_stream_base<boost::uint8_t *> >
		{
			typedef buffer_stream< unchecked_buffer_stream_base<boost::uint8_t *> > base_type;

			template<typename T>
			unchecked_buffer_stream_adapter(std::vector<T> & v)
				: base_type(&v[0], &v[0] + v.size())
			{}

			template<typename T, size_t N>
			unchecked_buffer_stream_adapter(boost::array<T,N> & a)
				: base_type(a.begin(), a.end())
			{}
		};

		struct buffer_stream_adapter
			: buffer_stream< checked_buffer_stream_base<boost::uint8_t *> >
		{
			typedef buffer_stream< checked_buffer_stream_base<boost::uint8_t *> > base_type;

			template<typename T>
			buffer_stream_adapter(std::vector<T> & v)
				: base_type()
			{
				if(!v.empty())
				{
					this->reset(&v[0], &v[0]+v.size());
				}
			}

			template<typename T, size_t N>
			buffer_stream_adapter(boost::array<T,N> & a)
				: base_type(a.begin(), a.end())
			{}
		};

		template<typename T>
		unchecked_buffer_stream_adapter adapt_unchecked(T & buffer){
			return unchecked_buffer_stream_adapter(buffer);
		}

		template<typename T>
		buffer_stream_adapter adapt_checked(T & buffer){
			return unchecked_buffer_stream_adapter(buffer);
		}
	}
}

#endif //GUARD_NET_CLIENT_UTILS_BUFFER_HPP_INCLUDED
