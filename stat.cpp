#ifndef STAT_INCLUDED
#define STAT_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"

struct stat_t
{
	uint num;
	lint max;
	lint min;
	lint avg;
	inline stat_t operator +( stat_t const& s ) const
	{
		return( stat_t{ .num = num + s.num,
						.max = max + s.max,
						.min = min + s.min,
						.avg = avg + s.avg } );
	}
	inline stat_t operator /( uint i ) const
	{
		return( stat_t{ .num = num / i,
						.max = max / i,
						.min = min / i,
						.avg = avg / i } );
	}
};

struct calc_stat_t : public stat_t
{
	constexpr calc_stat_t() :
		stat_t{ .num = 0,
			    .max = type_t<lint>::min(),
			    .min = type_t<lint>::max(),
			    .avg = 0 }
	{ }
	void add( lint const& val )
	{
		num++;
		if( max < val )
			max = val;
		if( min > val )
			min = val;
		avg += val;
	}
	void add( stat_t const& s )
	{
		num++;
		if( max < s.max )
			max = s.max;
		if( min > s.min )
			min = s.min;
		avg += s.avg;
	}
	stat_t stat() const
	{
		return( stat_t{ .num = num,
			            .max = num ? max : 0,
			            .min = num ? min : 0,
			            .avg = num ? avg / num : 0 } );
	}
};

template <uint n>
struct stats_t
{
	stat_t data[n];
	uint slot;
	constexpr stats_t( uint _slot = 0 ) :
		slot( _slot )
	{ }
	inline stats_t operator +( stats_t const& s ) const
	{
		stats_t o( slot + s.slot );
		for( uint i = 0; i < n; i++ )
			o.data[i] = data[i] + s.data[i];
		return( o );
	}
	inline stats_t operator /( uint i ) const
	{
		stats_t o( slot / i );
		for( uint i = 0; i < n; i++ )
			o.data[i] = data[i] / i;
		return( o );
	}
};

template <uint n>
struct calc_stats_t
{
	calc_stat_t data[n];
	uint slot;
	constexpr calc_stats_t( uint _slot = 0 ) :
		slot( _slot )
	{ }
	stats_t<n> stats() const
	{
		stats_t<n> s( slot );
		for( uint i = 0; i < n; i++ )
			s.data[i] = data[i].stat();
		return( s );
	}
	void add( lint const(& _data)[n] )
	{
		for( uint i = 0; i < n; i++ )
			data[i].add( _data[i] );
	}
};

#endif // STAT_INCLUDED

