#ifndef CRC_INCLUDED
#define CRC_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"

template <typename t>
class crc_t
{
protected:
	constexpr static uint crc_tab_len = 1 << type_t<byte>::numbits();
	
	t r;
	t poly;
	t xin;
	t xout;
	bool ref;
	t tab[crc_tab_len];
	
public:
	crc_t() = default;
	
	crc_t( t _poly, t _xin, t _xout, bool _ref )
	{
		ref = _ref;
		if( ref )
		{
			poly = reflect( _poly );
			xin = reflect( _xin );
			xout = reflect( _xout );
			crc_le_table( &tab, poly );
		} else {
			poly = _poly;
			xin = _xin;
			xout = _xout;
			crc_be_table( &tab, poly );
		}
		reset();
	}

	void reset()
	{
		r = xin;
	}
	
	t get() const
	{
		return( r ^ xout );
	}
	
	void add( byte c )
	{
		if( ref )
			r = crc_lsb_step( &tab, r, c );
		else
			r = crc_msb_step( &tab, r, c );
	}
	
	t calc( byte const * buf, uint len )
	{
		reset();
		for( uint i = 0; i < len; i++ )
			add( buf[i] );
		return( get() );
	}
	
	t get_poly() const
	{
		return( ref ? reflect( poly ) : poly );
	}

	uint get_size() const
	{
		return( sizeof( t ) );
	}

	uint get_tab_len() const
	{
		return( crc_tab_len );
	}

	t get_tab_val( uint i ) const
	{
		return( tab[i] );
	}


protected:	
	static void crc_be_table( t (* tab)[crc_tab_len], t poly )
	{
		constexpr t highbit = (t)1 << ( type_t<t>::numbits() - 1 );
		t crc = highbit;
		(*tab)[0] = 0;
		uint i = 1;
		while( i < lenof( (*tab) ) )
		{
			if( crc & highbit )
				crc = ( crc << 1 ) ^ poly;
			else
				crc = crc << 1;
			for( uint j = 0; j < i; j++ )
				(*tab)[i + j] = crc ^ (*tab)[j];
			i = i << 1;
		}
	}

	static void crc_le_table( t (* tab)[crc_tab_len], t poly )
	{
		t crc = 1;
		(*tab)[0] = 0;
		uint i = crc_tab_len / 2;
		while( i )
		{
			if( crc & 1 )
				crc = ( crc >> 1 ) ^ poly;
			else
				crc = crc >> 1;
			for( uint j = 0; j < crc_tab_len; j += 2 * i )
				(*tab)[i + j] = crc ^ (*tab)[j];
			i = i >> 1;
		}
	}

	constexpr static t crc_msb_step( t const(* tab)[crc_tab_len], t r, byte c )
	{
		constexpr uint bl = type_t<byte>::numbits();
		constexpr uint bl1 = ( sizeof(t) - 1 ) * type_t<byte>::numbits();
		return( ( r << bl ) ^ (*tab)[c ^ ( r >> bl1 )] );
	}

	constexpr static t crc_lsb_step( t const(* tab)[crc_tab_len], t r, byte c )
	{
		constexpr uint bl = type_t<byte>::numbits();
		constexpr uint bm = type_t<byte>::max();
		return( ( r >> bl ) ^ (*tab)[c ^ ( r & bm )] );
	}
	
	static t reflect( t v )
	{
		t u = 0;
		t i = 1;
		t j = (t)1 << ( sizeof(t) * type_t<byte>::numbits() - 1 );
		while( j )
		{
			if( v & j )
				u |= i;
			i <<= 1;
			j >>= 1;
		}
		return( u );
	}
};
template <typename t> constexpr uint crc_t<t>::crc_tab_len;

#endif // CRC_INCLUDED

