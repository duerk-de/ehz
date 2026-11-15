#ifndef EMPLACE_INCLUDED
#define EMPLACE_INCLUDED

#include "db_defines.cpp"
#include <utility>

namespace emplace_t
{
	template <typename t>
	inline t * alloc( uint const& len )
	{
		return( static_cast<t*>( len ? operator new[]( len * sizeof(t) ) : nullptr ) );
	}
	
	template <typename t>
	inline void free( t *& data )
	{
		if( !data )
			return;
		operator delete[]( data );
		data = nullptr;
	}
	
	template <typename t>
	constexpr inline void construct( t * const& var, uint const& i, t && val )
	{
		new( &var[i] ) t( std::move( val ) );
	}
	
	template <typename t>
	constexpr inline void destruct( t * const& var, uint const& i )
	{
		var[i].~t();
	}
};

#endif // EMPLACE_INCLUDED

