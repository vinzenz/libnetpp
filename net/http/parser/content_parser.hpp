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
#ifndef GUARD_NET_HTTP_PARSER_CONTENT_PARSER_HPP_INCLUDED
#define GUARD_NET_HTTP_PARSER_CONTENT_PARSER_HPP_INCLUDED

#include <net/http/detail/traits.hpp>
#include <boost/foreach.hpp>

namespace net
{
	namespace http
	{
		template<typename Tag>
		class basic_content_parser
		{
		public:
			basic_content_parser()
			{
			}
		};

		template<typename Tag>
		class basic_chunked_content_parser
					: public basic_content_parser<Tag>
		{
			enum parse_state_t
			{
				FAIL_STATE,
				PARSE_CHUNK_SIZE_START, PARSE_CHUNK_SIZE,
				PARSE_EXPECTING_CR_AFTER_CHUNK_SIZE,
				PARSE_EXPECTING_LF_AFTER_CHUNK_SIZE, PARSE_CHUNK,
				PARSE_EXPECTING_CR_AFTER_CHUNK, PARSE_EXPECTING_LF_AFTER_CHUNK,
				PARSE_EXPECTING_FINAL_CR_AFTER_LAST_CHUNK,
				PARSE_EXPECTING_FINAL_LF_AFTER_LAST_CHUNK
			};
			typedef typename chunk_cache_traits<Tag>::type chunk_cache_type;
			typedef parser_traits<Tag> traits_type;
			typedef typename traits_type::char_type char_type;

			parse_state_t state_;
			chunk_cache_type chunk_cache_;
			typename chunk_cache_type::value_type current_chunk_;
			std::string chunk_size_str_;
			std::size_t chunk_size_;

		public:
			basic_chunked_content_parser()
					: state_( PARSE_CHUNK_SIZE_START )
					, chunk_cache_()
					, current_chunk_()
					, chunk_size_str_()
					, chunk_size_(0)
			{

			}

			bool valid() const
			{
				return state_ != FAIL_STATE;
			}

			void clear()
			{
				state_ = PARSE_CHUNK_SIZE_START;
				chunk_cache_.clear();
				current_chunk_.clear();
				chunk_size_str_.clear();
				chunk_size_ = 0;
			}


			template<typename InputIterator>
			boost::tribool parse( InputIterator & iter, InputIterator end, basic_message<Tag> & message )
			{
				boost::tribool result = parse_impl( iter, end, message );
				if ( result == true )
				{
					typedef typename chunk_cache_type::value_type cache_type;
					BOOST_FOREACH(cache_type const & c, chunk_cache_)
					{
						message.body().insert(message.body().end(), c.begin(), c.end());
					}
				}
				return result;
			}
		private:

			template<parse_state_t TrueState>
			inline bool conditional_state( bool condition )
			{
				if ( condition )
				{
					state_ = TrueState;
				}
				else
				{
					state_ = FAIL_STATE;
				}
				return condition;
			}


			template<typename InputIterator>
			boost::tribool parse_impl( InputIterator & iter, InputIterator end, basic_message<Tag> & )
			{
				while ( iter != end )
				{
					char_type c = *iter;
					switch ( state_ )
					{
					case PARSE_CHUNK_SIZE_START:
						{
							if ( conditional_state<PARSE_CHUNK_SIZE>( traits_type::is_hex_digit( c ) ) )
							{
								chunk_size_str_.clear();
								chunk_size_str_.push_back( c );
							}
							else
							{
								conditional_state<PARSE_CHUNK_SIZE_START>( c == ' ' || c == '\t' || c == '\r' || c == '\n' );
							}
						}
						break;
					case PARSE_CHUNK_SIZE:
						if ( !conditional_state<PARSE_CHUNK_SIZE>( traits_type::is_hex_digit( c ) ) )
						{
							if ( !conditional_state<PARSE_EXPECTING_LF_AFTER_CHUNK_SIZE>( c == '\r' ) )
							{
								conditional_state<PARSE_EXPECTING_CR_AFTER_CHUNK_SIZE>( c == ' ' || c == '\t' );
							}
						}
						else
						{
							chunk_size_str_.push_back(c);
						}
						break;
					case PARSE_EXPECTING_CR_AFTER_CHUNK_SIZE:
						if ( !conditional_state<PARSE_EXPECTING_LF_AFTER_CHUNK_SIZE>( c == '\r' ) )
						{
							conditional_state<PARSE_EXPECTING_CR_AFTER_CHUNK_SIZE>( c == ' ' || c == '\t' );
						}
						break;
					case PARSE_EXPECTING_LF_AFTER_CHUNK_SIZE:
						if ( conditional_state<PARSE_CHUNK>( c == '\n' ) )
						{
							chunk_size_ = strtol( chunk_size_str_.c_str(), 0, 16 );
							if ( !chunk_size_ )
							{
								state_ = PARSE_EXPECTING_FINAL_CR_AFTER_LAST_CHUNK;
							}
							else
							{
								current_chunk_.clear();
								current_chunk_.reserve(chunk_size_);
							}
						}
						break;
					case PARSE_CHUNK:
						if(conditional_state<PARSE_CHUNK>(current_chunk_.size() < chunk_size_))
						{
							current_chunk_.push_back(c);
						}
						if(current_chunk_.size() == chunk_size_)
						{
							chunk_cache_.push_back(typename chunk_cache_type::value_type());
							chunk_cache_.back().swap(current_chunk_);
							state_ = PARSE_EXPECTING_CR_AFTER_CHUNK;
						}
						break;
					case PARSE_EXPECTING_CR_AFTER_CHUNK:
						conditional_state<PARSE_EXPECTING_LF_AFTER_CHUNK>(c == '\r');
						break;
					case PARSE_EXPECTING_LF_AFTER_CHUNK:
						conditional_state<PARSE_CHUNK_SIZE_START>(c == '\n');
						break;
					case PARSE_EXPECTING_FINAL_CR_AFTER_LAST_CHUNK:
						conditional_state<PARSE_EXPECTING_FINAL_LF_AFTER_LAST_CHUNK>(c == '\r');					
						break;
					case PARSE_EXPECTING_FINAL_LF_AFTER_LAST_CHUNK:
						if(conditional_state<PARSE_CHUNK_SIZE_START>(c == '\n'))
						{
							++iter;
							return true;
						}
						break;
					case FAIL_STATE:
						return false;

					default:
						assert( false && "Unknown state received" );
						state_ = FAIL_STATE;
						return false;
					}

					if ( state_ == FAIL_STATE )
					{
						return false;
					}

					++iter;
				}
				
				return boost::indeterminate;
			}
		};
	}
}

#endif //GUARD_NET_HTTP_PARSER_CONTENT_PARSER_HPP_INCLUDED
