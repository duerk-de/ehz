#ifndef RINGBUF_INCLUDED
#define RINGBUF_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "ctrl.cpp"
#include "emplace.cpp"


/** implementation of a ring buffer
 *  designed for continuously adding (complex) data
 *  and to read out last added data first
 *  data is overwritten if full
 *  last added data is stored at pos
 *  data is valid up to max and wrapping around to 0 if full
 */
template <typename t>
class ringbuf_t : public not_copyable_t
{
protected:
	t * data;
	uint max;
	uint pos;
	bool full;

public:
	constexpr ringbuf_t() :
		data( nullptr ),
		max( 0 ),
		pos( max ),
		full( false )
	{ }
	
	ringbuf_t( uint maxnum ) :
		data( nullptr ),
		max( 0 ),
		pos( max ),
		full( false )
	{
		resize_truncfirst( maxnum );
	}
	
	~ringbuf_t()
	{
		for( uint i = full ? 0 : pos; i < max; i++ )
			emplace_t::destruct( data, i );
		emplace_t::free( data );
	}
	
	uint const& maxnum() const
	{
		return( max );
	}
	
	void resize_truncfirst( uint maxnum )
	{
		if( maxnum != max )
		{
			t * old_data = data;
			uint old_pos = pos;
			uint old_max = max;
			uint old_full = full;
			max = maxnum;
			data = emplace_t::alloc<t>( max );
			ENSURE( data, "linearbuf alloc error for %y bytes\n", (double)( max * sizeof(t) ) );
			uint old_num = old_full ? old_max : old_max - old_pos;
			if( old_num >= max )
			{
				for( uint i = 0; i < max; i++ )
				{
					emplace_t::construct( data, i, std::move( old_data[old_pos] ) );
					emplace_t::destruct( old_data, old_pos );
					if( ++old_pos >= old_max )
						old_pos -= old_max;
				}
				for( uint i = max; i < old_num; i++ )
				{
					emplace_t::destruct( old_data, old_pos );
					if( ++old_pos >= old_max )
						old_pos -= old_max;
				}
				pos = 0;
				full = true;
			} else {
				for( uint i = max - old_num; i < max; i++ )
				{
					emplace_t::construct( data, i, std::move( old_data[old_pos] ) );
					emplace_t::destruct( old_data, old_pos );
					if( ++old_pos >= old_max )
						old_pos -= old_max;
				}
				pos = max - old_num;
				full = false;
			}
			emplace_t::free( old_data );
		}
	}
	
	inline void clear()
	{
		uint b = max;
		uint e = full ? 0 : pos;
		while( b > e )
			emplace_t::destruct( data, --b );
		pos = max;
		full = false;
	}
	
	
	inline bool const& isfull() const
	{
		return( full );
	}
	
	inline uint num() const
	{
		return( full ? max : max - pos );
	}
	
	inline t & operator []( uint p )
	{
		p += pos;
		if( p >= max )
			p -= max;
		return( data[p] );
	}
	
	inline t const& operator []( uint p ) const
	{
		p += pos;
		if( p >= max )
			p -= max;
		return( data[p] );
	}
	
	void add( t const& v )
	{
		if( !max )
			return;
		if( !pos )
		{
			pos = max;
			if( !full )
				full = true;
		}
		--pos;
		if( full )
			data[pos] = v;
		else
			emplace_t::construct( data, pos, v );
	}
	
	void add( t && v )
	{
		if( !max )
			return;
		if( !pos )
			pos = max;
		--pos;
		if( full )
		{
			data[pos] = std::move( v );
		} else {
			emplace_t::construct( data, pos, std::move( v ) );
			if( !pos )
				full = true;
		}
	}
	
	void reverse()
	{
		uint b = pos;
		uint e = full ? pos : max;
		uint l = ( full ? max : max - pos ) / 2;
		while( l-- )
		{
			if( !e )
				e = max;
			--e;
			t tmp = std::move( data[b] );
			data[b] = std::move( data[e] );
			data[e] = std::move( tmp );
			if( ++b >= max )
				b = 0;
		}
	}
};

#endif // RINGBUF_INCLUDED

