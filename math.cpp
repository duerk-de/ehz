#ifdef MATH_INCLUDING
#error  MATH_INCLUDING
#endif //  MATH_INCLUDING
#define MATH_INCLUDING
#ifndef MATH_INCLUDED
#define MATH_INCLUDED

#include "ctrl.cpp"

namespace math
{
/*
	template <typename t>
	static t sqrt_heron_dbg( t value, t estimated )
	{
		d_fprint_err( "sqrt_heron value: %U estimated: %U\n",
					  (ulint)value, (ulint)estimated );
		if( estimated == 0 )
			estimated = 1;
		if( value == (t)0 )
			return( (t)0 );
		t check = estimated;
		estimated = ( estimated + check ) / (t)2;
		check = value / estimated;
		d_fprint_err( "sqrt_heron check: %U estimated: %U\n",
					  (ulint)check, (ulint)estimated );
		t last;
		uint i = 0;
		do {
			last = estimated;
			estimated = ( estimated + check ) / (t)2;
			check = value / estimated;
			d_fprint_err( "sqrt_heron check: %U estimated: %U last: %U\n",
						  (ulint)check, (ulint)estimated, (ulint)last );
			if( ++i > 4 )
				break;
		} while( estimated != last && check != last );
		return( estimated );
	}
*/
	template <typename t>
	static t sqrt_heron( t value, t estimated )
	{
		if( estimated == 0 )
			estimated = 1;
		t be = estimated;
		if( value == (t)0 )
			return( (t)0 );
		t check = estimated;
		estimated = ( estimated + check ) / (t)2;
		check = value / estimated;
		t last;
		uint i = 0;
		do {
			last = estimated;
			estimated = ( estimated + check ) / (t)2;
			check = value / estimated;
			if( ++i > 24 )
			{
				d_fprint_err( "sqrt_heron i: %2i value: %U estimated: %U > %U\n",
				              i, (lint)value, (lint)be, (lint)estimated );
				if( i > 32 )
				{
					d_fprint_err( "sqrt_heron failed\n" );
					break;
				}
			}
		} while( estimated != last && check != last );
		return( estimated );
	}
	
	template <typename t>
	static t sqrt_babylon( t a, t b )
	{
		a = d_abs( a );
		b = d_abs( b );
		if( a < b )
			d_flip( a, b );
		t a2 = a * a;
		t b2 = b * b;
		t n = a2 + b2;
		t x = a + b2 / ( (t)2 * a );
		return( sqrt_heron( n, x ) );
	}
	
};

#endif // MATH_INCLUDED
#undef MATH_INCLUDING

