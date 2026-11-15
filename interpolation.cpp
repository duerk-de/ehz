#ifndef INTERPOLATION_INCLUDED
#define INTERPOLATION_INCLUDED

#include "ctrl.cpp"
#include "db_defines.cpp"
#include "db_mem.cpp"

class iterator_up_t
{
protected:
	uint end;
	uint curr;
	uint prev;
public:
	inline constexpr iterator_up_t( uint _end, uint _beg = 0 ) :
		end( _end ),
		curr( _beg ),
		prev( curr )
	{ }
	inline bool inrange() const
	{
		return( curr < end );
	}
	bool next()
	{
		if( !inrange() )
			return( false );;
		prev = curr;
		curr++;
		if( !inrange() )
		{
			prev = curr;
			return( false );
		}
		return( true );
	}
	inline uint const& lo() const
	{
		return( prev );
	}
	inline uint const& hi() const
	{
		return( curr );
	}
};

struct iterator_down_t
{
protected:
	uint beg;
	uint end;
	uint curr;
	uint prev;
public:
	inline constexpr iterator_down_t( uint _end, uint _beg = 0 ) :
		beg( _beg ),
		end( _end ),
		curr( end ? ( beg > end ? end : end - 1 ) : end ),
		prev( curr )
	{ }
	inline bool inrange() const
	{
		return( curr < end );
	}
	bool next()
	{
		if( !inrange() )
			return( false );
		if( curr <= beg )
		{
			curr = end;
			prev = curr;
			return( false );
		}
		prev = curr;
		curr--;
		return( true );
	}
	inline uint const& lo() const
	{
		return( curr );
	}
	inline uint const& hi() const
	{
		return( prev );
	}
};

template <typename t>
class ipol_pt_t
{
protected:
	uint l;
	t v_lo;
	t v_hi;
	t v_z;
public:
	enum type_t { high, low, increasing, decreasing };
	inline constexpr ipol_pt_t( uint _l, t _v_lo, t _v_hi, t _v_z ) :
		l( _l ),
		v_lo( _v_lo ),
		v_hi( _v_hi ),
		v_z( _v_z )
	{ }
	inline uint const& lo() const
	{
		return( l );
	}
	inline uint hi() const
	{
		return( l + 1 );
	}
	inline void increase_position()
	{
		l++;
	}
	type_t type() const
	{
		if( v_lo < v_z )
			return( v_hi < v_z ? low : increasing );
		else
			return( v_hi < v_z ? decreasing : high );
	}
	static constexpr bool valid( type_t const& typ )
	{
		switch( typ )
		{
			case high:
			case low:
				return( false );
			case increasing:
			case decreasing:
				return( true );
			default:
				ENSURE( false, "ipol_pt_t::type()" );
				return( false );
		}
	}
	template <typename c_t>
	c_t val( c_t v1, c_t v2 ) const
	{
		return( v1 + ( v2 - v1 ) * ( v_z - v_lo ) / ( v_hi - v_lo ) );
	}
};

#endif // INTERPOLATION_INCLUDED

