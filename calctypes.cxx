#ifndef CALCTYPES_INCLUDED
#define CALCTYPES_INCLUDED


	struct text_t
	{
		char const* str;
		uint len;
		text_t() :
			str( nullptr ),
			len( 0 )
		{ }
		text_t( char const* _str )
		{
			str = _str;
			len = d_strlen( _str );
		}
		text_t( char const* _str, uint _len )
		{
			str = _str;
			len = _len;
		}
		static text_t range( char const* _str, uint beg, uint end )
		{
			ENSURE( beg <= end, "text_t argument error\n" );
			return( text_t( _str + beg, end - beg ) );
		}
	};
	
	struct subtext_t : public text_t
	{
		uint off;
		uint tot;
		subtext_t() :
			off( 0 ),
			tot( 0 )
		{ }
		subtext_t( text_t text ) :
			text_t( text ),
			off( 0 ),
			tot( text.len )
		{ }
		subtext_t( text_t text, uint beg, uint end ) :
			text_t( text.str + beg, end - beg ),
			off( beg ),
			tot( text.len )
		{
			ENSURE( beg <= end, "subtext_t argument error\n" );
			ENSURE( end <= text.len, "subtext_t argument error\n" );
		}
		subtext_t( subtext_t text, uint beg, uint end ) :
			text_t( text.str + beg, end - beg ),
			off( beg + text.off ),
			tot( text.tot )
		{
			ENSURE( beg <= end, "subtext_t argument error\n" );
			ENSURE( end <= text.tot, "subtext_t argument error\n" );
		}
		text_t total() const
		{
			return( text_t( str - off, tot ) );
		}
		bool operator ==( subtext_t t ) const
		{
			return( str == t.str &&
			        len == t.len &&
			        off == t.off &&
			        tot == t.tot );
		}
	};

	template <uint maxnum, typename t>
	struct ptr_arr_t
	{
		uint num;
		t * arr[maxnum];
		ptr_arr_t() :
			num( 0 )
		{
			for( uint i = 0; i < maxnum; i++ )
				arr[i] = nullptr;
		}
		~ptr_arr_t()
		{
			drop();
		}
		ptr_arr_t( ptr_arr_t const& p ) = delete;
		ptr_arr_t( ptr_arr_t && p )
		{
			num = p.num;
			cpy( arr, p.arr );
			p.num = 0;
			zro( p.arr, sizeof( p.arr ) );
		}
		ptr_arr_t & operator =( ptr_arr_t const& p ) = delete;
		ptr_arr_t & operator =( ptr_arr_t && p )
		{
			drop();
			num = p.num;
			cpy( arr, p.arr );
			p.num = 0;
			zro( p.arr, sizeof( p.arr ) );
			return( *this );
		}
		void drop()
		{
			for( uint i = 0; i < num; i++ )
			{
				delete arr[i];
				arr[i] = nullptr;
			}
			num = 0;
		}
		void add( t * ptr )
		{
			ENSURE( ptr, "ptr_arr_t nullptr @ %i\n", num );
			ENSURE( num < maxnum, "ptr_arr_t overflow @ %i\n", num );
			arr[num++] = ptr;
		}
		void move( t *& ptr )
		{
			ENSURE( ptr, "ptr_arr_t nullptr @ %i\n", num );
			ENSURE( num < maxnum, "ptr_arr_t overflow @ %i\n", num );
			arr[num++] = ptr;
			ptr = nullptr;
		}
		inline t * operator []( uint i ) const
		{
			ENSURE( i < num, "ptr_arr_t access @ %i / %i\n", i, num );
			return( arr[i] );
		}
		inline t * last() const
		{
			ENSURE( num, "ptr_arr_t empty\n" );
			return( arr[num-1] );
		}
		bool has_nullptr() const
		{
			for( uint i = 0; i < num; i++ )
				if( !arr[i] )
					return( true );
			return( false );
		}
	};

	struct op_t
	{
		static constexpr char space = ' ';
		static constexpr char bbeg = '[';
		static constexpr char bend = ']';
		
		static inline bool varbegchar( char c )
		{
			return( ( c >= 'A' && c <= 'Z' ) ||
			        ( c >= 'a' && c <= 'z' ) ||
			        c == '_' );
		}
		
		static inline bool varchar( char c )
		{
			return( varbegchar( c ) ||
			        ( c >= '0' && c <= '9' ) );
		}
		
		enum type_t { none, right, left, middle } type;
		text_t const text;
		bool const invariant;
		op_t( text_t _text, type_t _type = middle, bool _invariant = true ) :
			type( _type ),
			text( _text ),
			invariant( _invariant )
		{ }
		virtual ~op_t() { }
		virtual calc_t calc( calc_t const(& args)[maxargs] ) const = 0;
	};


#endif // CALCTYPES_INCLUDED

