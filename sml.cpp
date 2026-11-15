#ifndef SML_INCLUDED
#define SML_INCLUDED

#include "db_mem.cpp"
#include "db_consoleio.cpp"
#include "parser.cpp"

namespace sml
{
	template <typename t>
	struct arr_ptr_t
	{
		uint num;
		t * items;

		constexpr arr_ptr_t() :
			num( 0 ),
			items( nullptr )
		{ }

		template <uint _num>
		constexpr arr_ptr_t( t (&_items)[_num] ) :
			num( _num ),
			items( &_items[0] )
		{ }

		constexpr arr_ptr_t( arr_ptr_t const& a ) :
			num( a.num ),
			items( a.items )
		{ }
		
		arr_ptr_t& operator =( arr_ptr_t const& a )
		{
			num = a.num,
			items = a.items;
			return( *this );
		}
		
		t& operator[]( uint i )
		{
			return( items[i] );
		}

		t const& operator[]( uint i ) const
		{
			return( items[i] );
		}
		
		bool isset() const
		{
			return( items );
		}
	};

	template <typename t>
	struct parsedval_t
	{
		t value;
		bool found;
		uint pos;
		uint len;
		
		constexpr parsedval_t() :
			value( 0 ),
			found( false ),
			pos( 0 ),
			len( 0 )
		{ }
		
		void reset()
		{
			*this = parsedval_t();
		}
		
		void set( t _value, uint _pos, uint _len )
		{
			value = _value;
			found = true;
			pos = _pos;
			len = _len;
		}
	};

	struct val_t
	{
		arr_ptr_t<uint const> name;
		parsedval_t<lint> parsed;
	};

	struct fnc_t {
		arr_ptr_t<byte const> name;
		arr_ptr_t<val_t> vals;
	};
	
	struct parsed_t
	{
		arr_ptr_t<fnc_t> fncs;
		
		void reset()
		{
			for( uint i = 0; i < fncs.num; i++ )
				for( uint j = 0; j < fncs[i].vals.num; j++ )
					fncs[i].vals[j].parsed.reset();
		}
		
		virtual bool evaluate( ulint timestamp ) = 0;
	};

	constexpr static uint marker[] = { 0x1b1b1b1bu, 0x01010101u };

	struct parser_t : public ::parser_t
	{
		constexpr static bool dbg = false;

		parsed_t & parsed;
		uint selected_stacklen;
		fnc_t * selected_fnc;
		
		constexpr parser_t( parsed_t & _parsed ) :
			parsed( _parsed ),
			selected_stacklen( 0 ),
			selected_fnc( nullptr )
		{ }
		
		virtual void setup()
		{
			parsed.reset();
			selected_stacklen = 0;
			selected_fnc = nullptr;
		}
		
		virtual bool teardown( ulint timestamp )
		{
			return( parsed.evaluate( timestamp ) );
		}
		
		struct stack_t
		{
			bool start;
			uint max;
			uint pos;
			constexpr stack_t() :
				start( false ),
				max( 0 ),
				pos( 0 )
			{ }
		};

		constexpr static uint maxstacklen = 8;

		enum datatype_t {
			dt_list, dt_str, dt_null, dt_empty, dt_int,
			dt_uint, dt_esc, dt_end, dt_unknown
		};
		
		ulint getunsigned( byte const* buf, uint len )
		{
			ulint val = 0;
			for( ulint i = 0; i < len; i++ )
				val = ( val << type_t<byte>::numbits() ) | buf[i];
			return( val );
		}

		lint getsigned( byte const* buf, uint len )
		{
			ulint u = getunsigned( buf, len );
			int l = sizeof( ulint ) - len;
			if( l < 0 )
				l = 0;
			l *= type_t<byte>::numbits();
			return( (lint)( u << l ) >> l );
		}

		void fileitem( datatype_t datatype, byte const* data, uint datalen,
					   stack_t * stack, uint stacklen, uint datapos )
		{
			if( dbg )
				printfileitem( datatype, data, datalen, stack, stacklen, datapos );
			
			if( selected_fnc && stacklen > 0 && stacklen - 1 < selected_stacklen )
			{
				selected_fnc = nullptr;
			}
			if( !selected_fnc )
			{
				if( stacklen > 0
					&& stack[stacklen-1].pos == stack[stacklen-1].max
					&& datatype == dt_str )
				{
					for( uint i = 0; i < parsed.fncs.num; i++ )
					{
						if( parsed.fncs[i].name.isset() && datalen
							== parsed.fncs[i].name.num 
							&& d_memcmp( data, &parsed.fncs[i].name[0],
										 parsed.fncs[i].name.num ) )
						{
							selected_fnc = &parsed.fncs[i];
							selected_stacklen = stacklen - 1;
							break;
						}
					}
					
				}
			}
			if( selected_fnc )
			{
				for( uint i = 0; i < selected_fnc->vals.num; i++ )
				{
					if( selected_fnc->vals[i].name.isset() )
					{
						bool found = stacklen - selected_stacklen 
									 == selected_fnc->vals[i].name.num;
						for( uint j = selected_stacklen; j < stacklen && found; j++ )
							if( stack[j].max - stack[j].pos
								!= selected_fnc->vals[i].name[j - selected_stacklen] )
								found = false;
						if( found )
						{
							switch( datatype )
							{
								case dt_int:
									selected_fnc->vals[i].parsed.set( getsigned( data, datalen ), datapos, datalen );
									break;
								case dt_uint:
									selected_fnc->vals[i].parsed.set( getunsigned( data, datalen ), datapos, datalen );
									break;
								default:
									selected_fnc->vals[i].parsed.set( 0, datapos, datalen );
									break;
							}
						}
					}
				}
			}
		}
		
		virtual bool parse( byte const* file, uint filelen, bool maybe_truncated )
		{
			bool okay = true;
			stack_t stack[maxstacklen];
			uint istack = 0;
			{
				uint n = 0xff;
				if( istack > lenof(stack) )
				{
					d_fprint_err( "stack overflow\n" );
					return( false );
				}
				stack[istack].start = true;
				stack[istack].max = n;
				stack[istack].pos = n;
				istack++;
			}
			byte const* endesc = (byte const*)&marker[0];
			uint endesclen = sizeof(marker[0]);
			
			uint i = sizeof(marker);
			while( i < filelen )
			{
				uint typ = ( file[i] & 0x70 ) >> 4;
				uint len = file[i] & 0x0f;
				datatype_t datatype;
				byte const* data = file + i + 1;
				int datalen = len - 1;
				switch( typ )
				{
					case 0x7:
						len = 1;
						datatype = dt_list;
						data = file + i;
						datalen = 1;
						break;
					case 0x0:
						if( len == 0 )
						{
							len = 1;
							datatype = dt_null;
							data = file + i;
							datalen = 0;
						} else {
							if( len == 1 )
							{
								datatype = dt_empty;
								data = file + i;
								datalen = 0;
							} else {
								datatype = dt_str;
								data = file + i + 1;
								datalen = len - 1;
							}
						}
						break;
					case 0x5:
						datatype = dt_int;
						data = file + i + 1;
						datalen = len - 1;
						break;
					case 0x6:
						datatype = dt_uint;
						data = file + i + 1;
						datalen = len - 1;
						break;
					case 0x1:
						datatype = dt_esc;
						if( len == 0xb )
						{
							len = filelen - i;
							data = file + i;
							datalen = len;
							if( datalen >= (int)endesclen * 2 && d_memcmp( data, endesc, endesclen ) )
							{
								datatype = dt_end;
								datalen = endesclen;
							}
						} else {
							data = file + i + 1;
							datalen = len - 1;
						}
						break;
					default:
						datatype = dt_unknown;
						data = file + i;
						datalen = len;
						break;
				}
				if( i + len > filelen )
				{
					if( !maybe_truncated )
					{
						d_fprint_err( "length error: %2,2,16u: %i+%i>%i\n", (uint)file[i], i, len, filelen );
						okay = false;
					}
					len = filelen - i;
					datalen = 0;
				}
				if( datalen < 0 )
					datalen = 0;
				
				fileitem( datatype, data, datalen, stack, istack, data - file );

				if( datatype == dt_list )
				{
					uint n = data[0] & 0x0f;
					if( istack > lenof(stack) )
					{
						d_fprint_err( "stack overflow\n" );
						okay = false;
						break;
					}
					stack[istack].start = true;
					stack[istack].max = n;
					stack[istack].pos = n;
					istack++;
				} else {
					while( istack > 0 )
					{
						if( stack[istack-1].pos > 0 )
							stack[istack-1].pos--;
						if( stack[istack-1].pos > 0 )
							break;
						istack--;
					}
				}
				if( datatype == dt_end )
					break;
				if( len == 0 )
				{
					d_fprint_err( "fixing zero length:\n" );
					len = 1;
				}
				i += len;
			}
			return( okay );
		}

		void printfileitem( datatype_t datatype, byte const* data, uint datalen,
							stack_t * stack, uint stacklen, uint datapos )
		{
			constexpr uint datadisplen = 12;

			switch( datatype )
			{
				case dt_list:
				case dt_null:
				case dt_empty:
				case dt_esc:
					break;
				case dt_str:
				case dt_int:
				case dt_uint:
				case dt_end:
				case dt_unknown:
				default:
				{
					d_fprint( "%4i ", datapos );
					for( uint i = 0; i < stacklen; i++ )
					{
						if( i == stacklen - 1 )
						{
							d_fprint( "%2,1i", stack[i].max - stack[i].pos );
						} else {
							if( i == stacklen - 2 && stack[stacklen-1].start )
							{
								stack[stacklen-1].start = false;
								d_fprint( "%2,1i", stack[i].max - stack[i].pos );
							} else {
								d_fprint( "  " );
							}
						}
					}
					for( uint i = stacklen; i < maxstacklen; i++ )
						d_fprint( "  " );
					cstr_t typestr;
					switch( datatype )
					{
						case dt_list:
							typestr = "list";
							break;
						case dt_str:
							typestr = "str ";
							break;
						case dt_null:
							typestr = "null";
							break;
						case dt_empty:
							typestr = "empt";
							break;
						case dt_int:
							typestr = "int ";
							break;
						case dt_uint:
							typestr = "uint";
							break;
						case dt_esc:
							typestr = "esc ";
							break;
						case dt_end:
							typestr = "end ";
							break;
						case dt_unknown:
						default:
							typestr = "unkn";
							break;
					}
					d_fprint( "%S  %*x", typestr, datalen, data );
					for( uint i = datalen; i < datadisplen; i++ )
						d_fprint( "  " );
					switch( datatype )
					{
						case dt_str:
							for( uint i = 0; i < datalen; i++ )
							{
								if( data[i] >= 0x20 && data[i] < 0x7f )
									d_fprint( "%c", data[i] );
								else
									d_fprint( "%c", '.' );
							}
							break;
						case dt_int:
							d_fprint( "%I", getsigned( data, datalen ) );
							break;
						case dt_uint:
							d_fprint( "%U", getunsigned( data, datalen ) );
							break;
						default:
							break;
					}
					d_fprint( "\n" );
					break;
				}
			}
		}
	};

	struct syncer_t : public ::syncer_t
	{
		virtual bool sync( uint * buf, uint & size )
		{
			byte * bytebuf = (byte *)buf;
			int markerpos = apos( bytebuf, 0, size,
			                      (byte const*)marker, sizeof(marker) );
			if( markerpos == 0 )
			{
				return( true );
			}
			if( markerpos < 0
				|| bytebuf[markerpos - 1] == (byte)( marker[0] & type_t<byte>::max() ) )
			{
				d_fprint_err( "out of sync\n" );
				cpy( bytebuf, bytebuf + size - sizeof(marker), sizeof(marker) );
				size = sizeof(marker);
				return( false );
			}
			d_fprint_err( "synced: %i + %i\n", markerpos, size - markerpos );
			d_fprint_err( "%*x\n", d_min( 1024u, size ), bytebuf );
			cpy( bytebuf, bytebuf + markerpos, size - markerpos );
			size -= markerpos;
			d_fprint_err( "%*x\n", d_min( 1024u, size ), bytebuf );
			return( true );
		}

		virtual bool findend( uint const* buf, uint size, uint & endpos )
		{
			uint pos = lenof(marker) - 1;
			uint len = size / sizeof(buf[0]) - 1;
			while( ++pos < len )
			{
				if( buf[pos] == marker[0]
					&& ( buf[pos+1] & type_t<byte>::max() ) == 0x1a
					&& buf[pos-1] != marker[0] )
				{
					endpos = pos + 2;
					return( true );
				}
			}
			return( false );
		}
	};
}

#endif // SML_INCLUDED

