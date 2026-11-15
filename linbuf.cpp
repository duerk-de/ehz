#ifndef LINBUF_INCLUDED
#define LINBUF_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "ctrl.cpp"
#include "emplace.cpp"


/** implementation of a contiguous linear buffer
 *  designed to hold a maximum of data as specified
 */
template <typename t>
class linbuf_t : public not_copyable_t
{
protected:
	t * data;
	uint max;
	uint pos;
	
public:
	inline constexpr linbuf_t() :
		data( nullptr ),
		max( 0 ),
		pos( 0 )
	{ }
	
	inline linbuf_t( uint maxnum ) :
		max( maxnum ),
		pos( 0 )
	{
		data = emplace_t::alloc<t>( max );
		ENSURE( data, "linearbuf alloc error for %y bytes\n", (double)( max * sizeof(t) ) );
	}
	
	inline ~linbuf_t()
	{
		for( uint i = 0; i < pos; i++ )
			emplace_t::destruct( data, i );
		emplace_t::free( data );
	}
	
	inline uint const& maxnum() const
	{
		return( max );
	}
	
	void trunclast( uint num )
	{
		while( pos > num )
			emplace_t::destruct( data, pos-- );
	}
	
	void resize( uint maxnum )
	{
		if( maxnum != max )
		{
			max = maxnum;
			t * old_data = data;
			uint old_pos = pos;
			ENSURE( old_pos <= max, "linearbuf new length of %i smaller than content length of %i\n", max, old_pos );
			data = emplace_t::alloc<t>( max );
			ENSURE( data, "linearbuf alloc error for %y bytes\n", (double)( max * sizeof(t) ) );
			for( pos = 0; pos < old_pos; pos++ )
			{
				emplace_t::construct( data, pos, std::move( old_data[pos] ) );
				emplace_t::destruct( old_data, pos );
			}
			emplace_t::free( old_data );
		}
	}
	
	void resize_trunclast( uint maxnum )
	{
		trunclast( maxnum );
		resize( maxnum );
	}
	
	inline void clear()
	{
		while( pos )
			emplace_t::destruct( data, pos-- );
	}
	
	inline uint const& num() const
	{
		return( pos );
	}
	
	inline constexpr t & operator []( uint p )
	{
		return( data[p] );
	}
	
	inline constexpr t const& operator []( uint p ) const
	{
		return( data[p] );
	}
	
	void add( t const& v )
	{
		ENSURE( pos < max, "linbuf_t length of %i exceeded\n", max );
		emplace_t::construct( data, pos++, v );
	}
	
	void add( t && v )
	{
		ENSURE( pos < max, "linbuf_t length of %i exceeded\n", max );
		emplace_t::construct( data, pos++, std::move( v ) );
	}
	
	void del()
	{
		ENSURE( pos, "linbuf_t length underflow\n" );
		emplace_t::destruct( (t *)data, --pos );
	}
	
	void shift( uint num )
	{
		ENSURE( num <= pos, "linbuf_t shift of %i exceeds number of elements of %i\n", num, pos );
		uint p = pos - num;
		for( uint i = 0; i < p; i++ )
			data[i] = std::move( data[i + num] );
		for( uint i = p + num; i < pos; i++ )
			emplace_t::destruct( (t *)data, i );
		pos = p;
	}
};

template <typename t, uint max>
class linarr_t : public not_copyable_t
{
protected:
	byte data[max * sizeof(t)];
	uint pos;
	
public:
	inline constexpr linarr_t() :
		pos( 0 )
	{ }
	
	inline ~linarr_t()
	{
		for( uint i = 0; i < pos; i++ )
			emplace_t::destruct( (t *)data, i );
	}
	
	inline static constexpr uint maxnum()
	{
		return( max );
	}
	
	inline void clear()
	{
		while( pos )
			emplace_t::destruct( (t *)data, pos-- );
	}
	
	inline uint num() const
	{
		return( pos );
	}
	
	inline constexpr t & operator []( uint p )
	{
		return( ((t *)data)[p] );
	}
	
	inline constexpr t const& operator []( uint p ) const
	{
		return( ((t *)data)[p] );
	}
	
	void add( t const& v )
	{
		ENSURE( pos < max, "linarr_t length of %i exceeded\n", max );
		emplace_t::construct( (t *)data, pos++, v );
	}
	
	void add( t && v )
	{
		ENSURE( pos < max, "linarr_t length of %i exceeded\n", max );
		emplace_t::construct( (t *)data, pos++, std::move( v ) );
	}
	
	void del()
	{
		ENSURE( pos, "linarr_t length underflow\n" );
		emplace_t::destruct( (t *)data, --pos );
	}
};


#endif // LINBUF_INCLUDED

