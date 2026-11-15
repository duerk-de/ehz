__GIT_SYMLINKS__
target=../_inc/db_math.cpp
target_resolved=/dibom/src/c/_inc/db_math.cpp
kind=file
sha256=ba387a92de76f7c00975327294bcd89f521bd4670ac2e74bd42b612619a0c24c
size=12696
created=2026-11-15T10:29:04.647617
--END-HEADER--
#ifndef DB_MATH_INCLUDED
#define DB_MATH_INCLUDED

#include "db_mem.cpp"

#define PI 3.141592653589793238462643383279502884197169399
#define E 2.71828182845904523536
#define SQR2H 0.7071067811865475

template <typename T> T d_sin45( T angle, T prec = 0.0 )
{
	T result, step, oldresult;
	int n;
	angle = (angle-45.0)/180.0*PI;
	n = 0;
	step = SQR2H;
	result = step;
	if( prec == 0.0 )
	{
		do {
			oldresult = result;
			step *= angle/(++n);
			result += step;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result += step;
		} while( result != oldresult );
	} else {
		do {
			step *= angle/(++n);
			result += step;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result += step;
		} while( step > prec );
	}
	return( result );
}

template <typename T> T d_cos45( T angle, T prec = 0.0 )
{
	T result, step, oldresult;
	int n;
	angle = (angle-45.0)/180.0*PI;
	n = 0;
	step = SQR2H;
	result = step;
	if( prec == 0.0 )
	{
		do {
			oldresult = result;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result += step;
			step *= angle/(++n);
			result += step;
		} while( result != oldresult );
	} else {
		do {
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result -= step;
			step *= angle/(++n);
			result += step;
			step *= angle/(++n);
			result += step;
		} while( step > prec );
	}
	return( result );
}

template <typename T> T d_sin0( T angle, T prec = 0.0 )
{
	T result, step, oldresult;
	int n;
	angle = angle/180.0*PI;
	n = 1;
	step = angle;
	result = step;
	if( prec == 0.0 )
	{
		do {
			oldresult = result;
			step *= (angle/(++n));
			step *= (angle/(++n));
			result -= step;
			step *= (angle/(++n));
			step *= (angle/(++n));
			result += step;
		} while( result != oldresult );
	} else {
		do {
			step *= (angle/(++n));
			step *= (angle/(++n));
			result -= step;
			step *= (angle/(++n));
			step *= (angle/(++n));
		} while( step > prec );
	}
	return( result );
}

template <typename T> T d_cos0( T angle, T prec = 0.0 )
{
	T result, step, oldresult;
	int n;
	angle = angle/180.0*PI;
	n = 0;
	step = 1.0;
	result = step;
	if( prec == 0.0 )
	{
		do {
			oldresult = result;
			step *= (angle/(++n));
			step *= (angle/(++n));
			result -= step;
			step *= (angle/(++n));
			step *= (angle/(++n));
			result += step;
		} while( result != oldresult );
	} else {
		do {
			step *= (angle/(++n));
			step *= (angle/(++n));
			result -= step;
			step *= (angle/(++n));
			step *= (angle/(++n));
			result += step;
		} while( step > prec );
	}
	return( result );
}

template <typename T> T d_sin( T angle, T prec = 0.0 )
{
	T postproc, a;
	postproc = (T)1.0;
	a = angle / (T)360.0;
	if( angle>(T)1e17  )
		return( (T)0 );
	if( a<(T)0.0 ) {
		angle *= (T)-1.0;
		a *= (T)-1.0;
		postproc *= (T)-1.0;
	}
	if( a>(T)1.0 ) {
		a = (T)(long long)a;
		angle -= a * (T)360.0;
	}
	if( angle<(T)0.0 )
		angle += (T)360.0;
	if( angle>(T)180.0 ) {
		angle -= (T)180.0;
		postproc *= (T)-1.0;
	}
	if( angle>(T)90.0 )
		angle = (T)180.0-angle;
	if( angle<(T)22.5 )
		return( postproc * d_sin0( angle, prec ) );
	else if( angle<67.5 )
		return( postproc * d_sin45( angle, prec ) );
	else
		return( postproc * d_cos0( (T)90.0-angle, prec ) );
}

template <typename T> inline T d_cos( T angle, T prec = 0.0 )
{
	return( d_sin( angle+(T)90.0, prec ) );
}

template <typename T> inline T d_tan( T angle, T prec = 0.0 )
{
	return( d_sin( angle, prec ) / d_cos( angle, prec ) );
}

template <typename T> inline T d_cot( T angle, T prec = 0.0 )
{
	return( 1.0 / d_tan( angle, prec ) );
}

template <typename T> T d_ln( T arg, T relres = 0.0 )
{
	T result, step, oldresult, postproc;
	int n, m;
	if( arg != arg )
		return( (T)0.0 );
	postproc = 1.0;
	if( arg>(T)1.0 ) {
		arg = (T)1.0/arg;
		postproc = (T)-1.0;
	}
	if( !( arg > (T)0.0 ) )
		return( (T)0.0 );
	for( m=0; arg<(T)0.54; arg*=E, m++ );
	arg = arg - (T)1.0;
	n = 1;
	step = arg;
	result = step;
	if( relres == (T)0.0 )
	{
		do {
			oldresult = result;
			step *= arg;
			result += -step/(++n);
			step *= arg;
			result += step/(++n);
		} while( result != oldresult && n < 300 );
	} else {
		do {
			step *= arg;
			result += -step/(++n);
			step *= arg;
			result += step/(++n);
		} while( step/result > relres );
	}
	return( postproc * ( result - (T)m ) );
}

template <typename T> T d_exp( T arg, T relres = 0.0 )
{
	T result, step, oldresult;
	int n;
	bool flip = false;
	if( arg != arg )
		return( (T)0.0 );
	if( arg<(T)0.0 ) {
		arg = -arg;
		flip = true;
	}
	if( arg>(T)707.0 )
		return( (T)0.0 );
	n = 0;
	step = (T)1.0;
	result = step;
	if( relres == (T)0.0 )
	{
		do {
			oldresult = result;
			step *= arg/(++n);
			result += step;
			step *= arg/(++n);
			result += step;
			step *= arg/(++n);
			result += step;
			step *= arg/(++n);
			result += step;
		} while( result != oldresult );
	} else {
		do {
			step *= arg/(++n);
			result += step;
			step *= arg/(++n);
			result += step;
			step *= arg/(++n);
			result += step;
			step *= arg/(++n);
			result += step;
		} while( step/result > relres );
	}
	if( flip )
		result = (T)1.0/result;
	return( result );
}

template <typename T> inline T d_hoch( T base, T exponent, T precision = 0 )
{
	if( !( base > (T)0 ) )
		return( (T)0 );
	else
		return( d_exp( d_ln( base, precision ) * exponent ) );
}

template <typename T> inline T d_sqrt( T arg, T prec = 0.0 )
{
	return( d_hoch( arg, (T)0.5, prec ) );
}

template <typename T> inline T d_log( T arg, T base, T prec = 0.0 )
{
	return( d_ln( arg, prec ) / d_ln( base, prec ) );
}

template <typename T> inline T d_lg( T arg, T prec = 0.0 )
{
	const static T ln10 = d_ln( 10.0 );
	return( d_ln( arg, prec ) / ln10 );
}

template <typename T> T d_arctan_mixed( T arg, T relres )
{
	T result, step;
	int n;
	if( arg < 1.16 )
	{
		if( arg < 1.0 )
		{
			result = 0.0;
		} else {
			result = 0.01333;
			step = arg;
			result += 0.88438 * step;
			step *= arg;
			result += 0.44208 * step;
			step *= arg;
			result -= 1.29949 * step;
			step *= arg;
			result += 1.29298 * step;
			step *= arg;
			result -= 0.82198 * step;
			step *= arg;
			result += 0.36983 * step;
			step *= arg;
			result -= 0.11708 * step;
			step *= arg;
			result += 0.02361 * step;
			step *= arg;
			result -= 0.00227 * step;
		}
	} else {
		if( relres==0.0 )
			relres = 1e-20;
		n = 1;
		arg = 1/arg;
		step = arg;
		result = PI*0.5 - step;
		do {
			step *= arg*arg;
			n += 2;
			result += step/n;
			step *= arg*arg;
			n += 2;
			result -= step/n;
		} while( step/n > relres && n < 40 );
	}
	return( result*180.0/PI );
}

template <typename T> T d_arctan( T arg, T relres = 1e-10 )
{
	T postproc, result;
	postproc = (T)1;
	if( arg < (T)0 ) {
		postproc = (T)-1;
		arg = -arg;
	}
	if( arg == (T)0 )
		result = (T)0;
	else if( arg < (T)1 )
		result = (T)90 - d_arctan_mixed( (T)1/arg, relres );
	else
		result = d_arctan_mixed( arg, relres );
	return( postproc * result );
}

template <typename T> T d_arcsin( T arg, T prec = 0.0 )
{
	int n;
	T step, result, arg2, oldstep;
	static int maxn = 1 << 16;

	if( arg > 1.0 || arg < -1.0 )
	{
		result = 0.0;
	} else {
		arg2 = arg*arg;
		step = arg;
		result = step;
		n = -1;
		if( prec == 0.0 )
		{
			do {
				oldstep = step;
				n+=2;
				step *= arg2*((T)n/(n+1))*((T)n/(n+2));
				result += step;
				n+=2;
				step *= arg2*((T)n/(n+1))*((T)n/(n+2));
				result += step;
				n+=2;
				step *= arg2*((T)n/(n+1))*((T)n/(n+2));
				result += step;
				n+=2;
				step *= arg2*((T)n/(n+1))*((T)n/(n+2));
				result += step;
				n+=2;
				step *= arg2*((T)n/(n+1))*((T)n/(n+2));
				result += step;
				n+=2;
				step *= arg2*((T)n/(n+1))*((T)n/(n+2));
				result += step;
			} while( step != oldstep && n < maxn );
		} else {
			if( arg < 0.0 )
			{
				prec = -prec;
				do {
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
				} while( step < prec && n < maxn );
			} else {
				do {
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
					n+=2;
					step *= arg2*((T)n/(n+1))*((T)n/(n+2));
					result += step;
				} while( step > prec && n < maxn );
			}
		}
	}
	return( result / PI * 180.0 );
}

template <typename T> inline T d_arccos( T arg, T prec = 0.0 )
{
	// return( 90.0 - d_arcsin( arg, prec ) );
	return( 90.0 - d_arctan( arg / d_sqrt((T)1-arg*arg,prec), prec ) );
}

template <typename T> T d_b10( T chi )
{
	T b10;
	if( chi > 2.6171504655 ) {
		b10 = 0.0;
	} else {
		if( chi > 1.4326566565 ) {
			b10 = 0.01831563889 * ( 141.0 + chi/3.0 * ( -640.0 + chi * ( 369.0 + chi * ( -96.0 + 9.5*chi ) ) ) );
		} else {
			if( chi > 0.55818262732 ) {
				b10 = 0.3678794412 * ( 2.5 + chi/3.0 * ( 4.0 + chi * ( -18.0 + chi * ( 12.0 - 2.5*chi ) ) ) );
			} else {
				chi *= chi;
				b10 = 1.0 - chi * ( 1.0 - chi * ( 0.5 - chi/6.0 ) );
			}
		}
	}
	return b10;
}

/*
template <typename T> inline T d_pwr( const T &base, const T &exponent, const T &precision = 0.0 )
{
	return( d_hoch( base, exponent, precision ) );
}
*/

template <typename d_t, typename j_t> inline d_t d_pwr( const d_t &d, const j_t &j )
{
	j_t k;
	d_t e = 1;
	for( k = 0; k < j; e *= d, k++ );
	return( e );
}




class complex
{
protected:
	double re, im;
	double va, ph;
	
public:
	complex()
	{
		re = 0.0;
		im = 0.0;
		va = 0.0;
		ph = 0.0;
	}

	complex( double _a1, double _a2, bool _isan = false )
	{
		if( _isan )
		{
			va = _a1;
			ph = _a2;
			antoka();
		} else {
			re = _a1;
			im = _a2;
			va = 0.0;
			ph = 0.0;
		}
	}
	
	static complex sqrt( double d )
	{
		if( d < 0.0 )
			return( complex( 0.0, d_sqrt(-d) ) );
		else
			return( complex( d_sqrt(d), 0.0 ) );
	}
	
	complex trirt()
	{
		if( d_abs( im/re ) < 1e-13 )
		{
			im = 0.0;
			if( re < 0.0 )
				return( complex( -d_hoch( -re, 1.0/3.0 ), 0.0 ) );
			else
				return( complex( d_hoch( re, 1.0/3.0 ), 0.0 ) );
		} else {
			return( up( 1.0/3.0 ) );
		}
	}
	
	inline void antoka()
	{
		re = va * d_cos( ph );
		im = va * d_sin( ph );
	}
	
	inline void katoan()
	{
		va = d_sqrt( re*re + im*im );
		if( re == 0.0 )
		{
			if( im == 0.0 )
				ph = 0.0;
			else if( im < 0.0 )
				ph = 270.0;
			else
				ph = 90.0;
		} else {
			ph = d_arctan( im / re );
			if( re < 0.0 )
				ph += 180;
			if( ph < 0.0 )
				ph += 360.0;
		}
	}
	
	inline double real()
	{
		return( re );
	}

	inline double imag()
	{
		return( im );
	}
	
	inline double value()
	{
		katoan();
		return( va );
	}

	inline double phase()
	{
		katoan();		
		return( ph );
	}
	
	inline complex up( double exponent )
	{
		katoan();
		if( ph > 180.0 )
			ph -= 360;
		return( complex( d_hoch( va, exponent ), ph * exponent, true ) );
	}
	
	inline complex operator + ( double d )
	{
		return( complex( re + d, im ) );
	}

	inline complex operator + ( complex c )
	{
		return( complex( re + c.re, im + c.im ) );
	}

	inline complex operator - ()
	{
		return( complex( -re, -im ) );
	}
	
	inline complex operator - ( double d )
	{
		return( complex( re - d, im ) );
	}

	inline complex operator - ( complex c )
	{
		return( complex( re - c.re, im - c.im ) );
	}

	inline complex operator * ( double d )
	{
		return( complex( re*d, im*d ) );
	}

	inline complex operator * ( complex c )
	{
		return( complex( re*c.re - im*c.im, re*c.im + im*c.re ) );
	}
	
};


#endif // DB_MATH_INCLUDED
