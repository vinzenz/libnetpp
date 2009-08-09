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
#ifndef GUARD_NET_HTTP_PARSER_HEADER_PARSER_HPP_INCLUDED
#define GUARD_NET_HTTP_PARSER_HEADER_PARSER_HPP_INCLUDED

#include <net/http/detail/tags.hpp>
#include <net/http/detail/traits.hpp>
#include <net/http/request/basic_request.hpp>
#include <net/http/response/basic_response.hpp>
#include <boost/logic/tribool.hpp>
#include <cassert>

namespace net
{
	namespace http
	{
		template<typename Tag, bool IsRequest>
		class basic_header_parser
		{
			enum parse_state_t
			{
				FAIL_STATE,

				PARSE_METHOD_START, PARSE_METHOD, PARSE_URI_STEM, PARSE_URI_QUERY,

				PARSE_HTTP_VERSION_H, PARSE_HTTP_VERSION_T_1, PARSE_HTTP_VERSION_T_2,
				PARSE_HTTP_VERSION_P, PARSE_HTTP_VERSION_SLASH,

				PARSE_HTTP_VERSION_MAJOR_START, PARSE_HTTP_VERSION_MAJOR,
				PARSE_HTTP_VERSION_MINOR_START, PARSE_HTTP_VERSION_MINOR,

				PARSE_STATUS_CODE_START, PARSE_STATUS_CODE, PARSE_STATUS_MESSAGE,

				PARSE_EXPECTING_NEWLINE, PARSE_EXPECTING_CR,

				PARSE_HEADER_WHITESPACE, PARSE_HEADER_START, PARSE_HEADER_NAME,
				PARSE_SPACE_BEFORE_HEADER_VALUE, PARSE_HEADER_VALUE,

				PARSE_EXPECTING_FINAL_NEWLINE, PARSE_EXPECTING_FINAL_CR
			};

			typedef typename string_traits<Tag>::type string_type;
			typedef std::pair<string_type, string_type> header_pair_type;
			typedef typename char_traits<Tag>::type  char_type;
			typedef parser_traits<Tag> traits_type;
			
			typedef typename boost::mpl::if_c<
							IsRequest,
							basic_request<Tag>,
							basic_response<Tag>
					>::type message_type;

			parse_state_t state_;
			header_pair_type header_pair_;
			message_type message_;
		public:
			basic_header_parser()
					: state_( IsRequest ? PARSE_METHOD_START : PARSE_HTTP_VERSION_H )
					, header_pair_()
					, message_()
			{
			}

			bool valid() const
			{
				return state_ != FAIL_STATE;
			}

			void clear()
			{
				state_ = IsRequest ? PARSE_METHOD_START : PARSE_HTTP_VERSION_H;
				message_ = message_type();
			}

			template<typename InputIterator>
			boost::tribool parse( InputIterator & iter, InputIterator end, message_type & request )
			{
				boost::tribool result = parse_impl( iter, end, message_ );
				if ( result == true )
				{
					request = message_;
					clear();
				}
				return result;
			}
		private:

			inline static bool is_valid_char( char_type c )
			{
				return traits_type::is_char( c ) && !traits_type::is_control( c ) && !traits_type::is_special( c );
			}

			inline void process_valid_hdr_char( char_type c )
			{
				if ( conditional_state<PARSE_HEADER_NAME>( is_valid_char( c ) ) )
				{
					header_pair_.first.clear();
					header_pair_.first.push_back( c );
				}
			}

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
			boost::tribool parse_values( InputIterator & iter, InputIterator end, basic_message<Tag> & message )
			{
				while ( iter != end )
				{
					switch ( state_ )
					{
					case PARSE_EXPECTING_NEWLINE:
						{
							char_type c = *iter;
							if ( !conditional_state<PARSE_HEADER_START>( c == '\n' ) )
							{
								if ( c == '\r' )
								{
									// we received two newlines in a row
									// assume newline only is (incorrectly) being used for line termination
									// therefore, the message is finished
									++iter;
									return true;
								}
								else
								{
									process_valid_hdr_char( c );
								}
							}
						}
						break;
					case PARSE_EXPECTING_CR:
						{
							char_type c = *iter;
							if ( !conditional_state<PARSE_HEADER_START>( c == '\r' ) )
							{
								if ( c == '\n' )
								{
									// we received two newlines in a row
									// assume newline only is (incorrectly) being used for line termination
									// therefore, the message is finished
									++iter;
									return true;
								}
								else if ( !conditional_state<PARSE_HEADER_WHITESPACE>( c == '\t' || c == ' ' ) )
								{
									process_valid_hdr_char( c );
								}
							}
						}
						break;

					case PARSE_HEADER_WHITESPACE:
						{
							char_type c = *iter;
							if ( !conditional_state<PARSE_EXPECTING_NEWLINE>( c == '\r' )
							        && !conditional_state<PARSE_EXPECTING_CR>( c == '\n' )
							        && !conditional_state<PARSE_HEADER_WHITESPACE>( c == '\t' || c == ' ' ) )
							{
								process_valid_hdr_char( c );
							}
						}
						break;
					case PARSE_HEADER_START:
						{
							char_type c = *iter;
							if ( !conditional_state<PARSE_EXPECTING_FINAL_NEWLINE>( c == '\r' )
							        && !conditional_state<PARSE_EXPECTING_FINAL_CR>( c == '\n' )
							        && !conditional_state<PARSE_HEADER_WHITESPACE>( c == '\t' || c == ' ' ) )
							{
								process_valid_hdr_char( c );
							}
						}
						break;
					case PARSE_HEADER_NAME:
						{
							char_type c = *iter;
							if ( conditional_state<PARSE_SPACE_BEFORE_HEADER_VALUE>( c == ':' ) )
							{
								header_pair_.second.clear();
							}
							else if ( conditional_state<PARSE_HEADER_NAME>( is_valid_char( c ) && header_pair_.first.size() < traits_type::HEADER_NAME_MAX ) )
							{
								header_pair_.first.push_back( c );
							}
						}
						break;
					case PARSE_SPACE_BEFORE_HEADER_VALUE:
						{
							char_type c = *iter;
							if ( !conditional_state<PARSE_HEADER_VALUE>( c == ' ' ) )
							{
								if ( conditional_state<PARSE_EXPECTING_NEWLINE>( c == '\r' )
								        || conditional_state<PARSE_EXPECTING_CR>( c == '\n' ) )
								{
									message.headers().insert( header_pair_ );
								}
								else if ( conditional_state<PARSE_HEADER_VALUE>( is_valid_char( c ) ) )
								{
									header_pair_.second.push_back( c );
								}
							}
						}
						break;
					case PARSE_HEADER_VALUE:
						{
							char_type c = *iter;
							if ( conditional_state<PARSE_EXPECTING_NEWLINE>( c == '\r' )
							        || conditional_state<PARSE_EXPECTING_CR>( c == '\n' ) )
							{
								message.headers().insert( header_pair_ );
							}
							else if ( conditional_state<PARSE_HEADER_VALUE>( !traits_type::is_control( c ) && header_pair_.second.size() < traits_type::HEADER_VALUE_MAX ) )
							{
								header_pair_.second.push_back( c );
							}
						}
						break;

					case PARSE_EXPECTING_FINAL_NEWLINE:
						if ( *iter == '\n' ) ++iter;
						return true;
					case PARSE_EXPECTING_FINAL_CR:
						if ( *iter == '\r' ) ++iter;
						return true;

					case FAIL_STATE:
						return false;

					default:
						assert( false && "Entered unknown state" ); // Unknown state
					}

					if ( state_ == FAIL_STATE )
					{
						return false;
					}

					++iter;
				}
				return true;
			}

			template<typename InputIterator>
			boost::tribool parse_impl( InputIterator & iter, InputIterator end, basic_request<Tag> & message )
			{
				while ( iter != end )
				{
					char_type c = *iter;
					switch ( state_ )
					{
					case PARSE_HTTP_VERSION_MINOR:
						if ( conditional_state<PARSE_HTTP_VERSION_MINOR> ( is_digit( c ) ) )
						{
							message.version().second = ( message.version().second * 10 ) + c - '0';
						}
						else if ( !conditional_state<PARSE_EXPECTING_NEWLINE> ( c == '\r' ) )
						{
							conditional_state<PARSE_EXPECTING_CR> ( c == '\n' );
						}
						break;

					case PARSE_METHOD_START:
						if ( c != ' ' && c != '\r' && c != '\n' )
						{
							if ( conditional_state( is_valid_char( c ) ) )
							{
								message.method().clear();
								message.method().push_back( c );
							}
						}
						break;
					case PARSE_METHOD:
						if ( !conditional_state<PARSE_URI_STEM>( c == ' ' ) )
						{
							if ( conditional_state<PARSE_METHOD>( is_valid_char( c ) && message.method().size() < traits_type::METHOD_MAX ) )
							{
								message.method().push_back( c );
							}
						}
						else
						{
							message.resource().clear();
						}
						break;
					case PARSE_URI_STEM:
						if ( !conditional_state<PARSE_HTTP_VERSION_H>( c == ' ' ) )
						{
							if ( conditional_state<PARSE_URI_QUERY>( c == '?' ) )
							{
								message.query().clear();
							}
							else if ( conditional_state<PARSE_URI_STEM>( !is_control( c ) && message.resource().size() < traits_type::RESOURCE_MAX ) )
							{
								message.resource().push_back( c );
							}
						}
						break;
					case PARSE_URI_QUERY:
						if ( !conditional_state<PARSE_HTTP_VERSION_H>( c == ' ' )
						        && conditional_state<PARSE_URI_QUERY>( !is_control( c ) && message.query().size() < traits_type::QUERY_STRING_MAX ) )
						{
							message.query().push_back( c );
						}
						break;
					case PARSE_HTTP_VERSION_H:
						{
							boost::tribool result = parse_version( iter, end, message );
							if ( result != boost::indeterminate )
							{
								return result;
							}
						}
					case FAIL_STATE:
						return false;
					default:
						return parse_values( iter, end, message );
					}

					if ( state_ == FAIL_STATE )
					{
						return false;
					}

					++iter;
				}
			}

			template<typename InputIterator>
			boost::tribool parse_version( InputIterator & iter, InputIterator end, basic_message<Tag> & message )
			{

				while ( iter != end )
				{
					char_type c = *iter;
					switch ( state_ )
					{
					case PARSE_HTTP_VERSION_H:
						conditional_state<PARSE_HTTP_VERSION_T_1> ( c == 'H' );
						break;
					case PARSE_HTTP_VERSION_T_1:
						conditional_state<PARSE_HTTP_VERSION_T_2> ( c == 'T' );
						break;
					case PARSE_HTTP_VERSION_T_2:
						conditional_state<PARSE_HTTP_VERSION_P> ( c == 'T' );
						break;
					case PARSE_HTTP_VERSION_P:
						conditional_state<PARSE_HTTP_VERSION_SLASH> ( c == 'P' );
						break;
					case PARSE_HTTP_VERSION_SLASH:
						conditional_state<PARSE_HTTP_VERSION_MAJOR_START> ( c == '/' );
						break;
					case PARSE_HTTP_VERSION_MAJOR_START:
						if ( conditional_state<PARSE_HTTP_VERSION_MAJOR> ( traits_type::is_digit( c ) ) )
						{
							message.version().first = c - '0';
						}
						break;
					case PARSE_HTTP_VERSION_MAJOR:
						if ( conditional_state<PARSE_HTTP_VERSION_MAJOR> ( traits_type::is_digit( c ) ) )
						{
							message.version().first = ( message.version().first * 10 ) + c - '0';
						}
						else
						{
							conditional_state<PARSE_HTTP_VERSION_MINOR_START> ( c == '.' );
						}
						break;
					case PARSE_HTTP_VERSION_MINOR_START:
						if ( conditional_state<PARSE_HTTP_VERSION_MINOR> ( traits_type::is_digit( c ) ) )
						{
							message.version().second = c - '0';
						}
						break;

					case FAIL_STATE:
						return false;

					default:
						return boost::indeterminate;
					}

					if ( state_ == FAIL_STATE )
					{
						return false;
					}

					++iter;
				}

				return true;
			}

			template<typename InputIterator>
			boost::tribool parse_impl( InputIterator & iter, InputIterator end, basic_message<Tag> & message )
			{
				boost::tribool result = parse_version( iter, end, message );
				if ( result != boost::indeterminate )
				{
					return result;
				}

				while ( iter != end )
				{
					char_type c = *iter;
					switch ( state_ )
					{
					case PARSE_HTTP_VERSION_MINOR:
						if ( conditional_state<PARSE_HTTP_VERSION_MINOR> ( traits_type::is_digit( c ) ) )
						{
							message.version().second = ( message.version().second * 10 ) + c - '0';
						}
						else
						{
							conditional_state<PARSE_STATUS_CODE_START> ( c == ' ' );
						}
						break;

					case PARSE_STATUS_CODE_START:
						if ( conditional_state<PARSE_STATUS_CODE> ( traits_type::is_digit( c ) ) )
						{
							message.status_code() = c - '0';
						}
						break;
					case PARSE_STATUS_CODE:
						if ( conditional_state<PARSE_STATUS_CODE> ( traits_type::is_digit( c ) ) )
						{
							message.status_code() = ( message.status_code() * 10 ) + c - '0';
						}
						else if ( conditional_state<PARSE_STATUS_MESSAGE> ( c == ' ' ) )
						{
							message.status_message().clear();
						}
						break;
					case PARSE_STATUS_MESSAGE:
						if ( !conditional_state<PARSE_EXPECTING_NEWLINE>( c == '\r' ) )
						{
							if ( !conditional_state<PARSE_EXPECTING_CR>( c == '\n' ) )
							{
								if ( conditional_state<PARSE_STATUS_MESSAGE>( !traits_type::is_control( c ) && message.status_message().size() < traits_type::STATUS_MESSAGE_MAX ) )
								{
									message.status_message().push_back( c );
								}
							}
						}
						break;

					case FAIL_STATE:
						return false;

					default:
						return parse_values( iter, end, message );
					}

					if ( state_ == FAIL_STATE )
					{
						return false;
					}

					++iter;
				}

				return true;
			}

		};


	}
}

#endif // GUARD_NET_HTTP_PARSER_HEADER_PARSER_HPP_INCLUDED
