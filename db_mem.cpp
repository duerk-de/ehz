__GIT_SYMLINKS__
target=../_inc/db_mem.cpp
target_resolved=/dibom/src/c/_inc/db_mem.cpp
kind=file
sha256=5cf9c00bc6442a4c910efd023a2494d2f448da3d0ce9f2521c69c18bf13849f1
size=35719
created=2026-11-16T20:49:12.745612
--END-HEADER--
#ifndef DB_MEM_INCLUDED
#define DB_MEM_INCLUDED

#include "db_defines.cpp"
#include <new>

#ifdef __OPTIMIZE__
#include <cstring>
#endif

#ifdef MAXINT
	#undef MAXINT
#endif
#ifdef MININT
	#undef MININT
#endif

#define lenof(arr) (sizeof(arr)/sizeof((arr)[0]))

template <typename t>
class type_t
{
protected:
	type_t() = delete;
	type_t( type_t const& ) = delete;
	type_t& operator =( type_t const& ) = delete;
public:
	constexpr static bool is_signed()
	{
		return( (t)-1 < (t)0 );
	}
	constexpr static bool is_unsigned()
	{
		return( (t)0 < (t)-1 );
	}
	constexpr static uint numbits()
	{
		return( sizeof( t ) * 8 );
	}
	constexpr static t setbits( uint num )
	{
		return( num ? ( setbits( num - 1 ) << 1 ) | (t)1 : (t)0 );
	}
	constexpr static t max()
	{
		return( is_signed() ? setbits( numbits() - 1 ) : (t)-1 );
	}
	constexpr static t min()
	{
		return( is_signed() ? (t)1 << ( numbits() - 1 ) : (t)0 );
	}
};

struct not_copyable_t
{
    not_copyable_t( not_copyable_t const& ) = delete;
    not_copyable_t& operator=( not_copyable_t const& ) = delete;
    not_copyable_t( not_copyable_t && ) = default;
    not_copyable_t& operator=( not_copyable_t && ) = default;
    not_copyable_t() = default;
};

struct not_movable_t : public not_copyable_t
{
    not_movable_t( not_movable_t const& ) = delete;
    not_movable_t& operator=( not_movable_t const& ) = delete;
    not_movable_t( not_movable_t && ) = delete;
    not_movable_t& operator=( not_movable_t && ) = delete;
    not_movable_t() = default;
};

template <typename t>
struct bin_t
{
	static constexpr t k =   (t)1 << 10;
	static constexpr t m = ( (t)1 << 10 ) * k;
	static constexpr t g = ( (t)1 << 10 ) * m;
};

template <typename i_t>
constexpr i_t iexp( i_t const& base, i_t const& exp )
{
	return( exp > 1 ? iexp( base, exp - 1 ) * base : exp > 0 ? base : 0 );
}


// ÷÷÷÷÷÷÷÷÷÷÷   basic variable operations   ÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷

/// returns minimum of two variables
template <typename t>
inline t d_min( t const& a, t const& b )
{
	return( b < a ? b : a );
}
/// returns maximum of two variables
template <typename t>
inline t d_max( t const& a, t const& b )
{
	return( b > a ? b : a );
}
/// returns absolute value of a variable
template <typename t>
inline t d_abs( t const& a )
{
	return( a < (t)0 ? -a : a );
}
/// exchanges content of two variables
template <typename t>
inline void d_flip( t & a, t & b )
{
	t c;
	c = a;
	a = b;
	b = c;
}
/// returns sqare value of a variable
template <typename t>
inline t d_sq( t const& a )
{
	return( a * a );
}
/// converts endian
template <typename t>
inline t flipendian( t const& differentendian )
{
	t r = 0;
	for( uint i = 0; i < sizeof(t); i++ )
		r |= (t)*( (byte*)differentendian + i ) << ((sizeof(t)-(i+1))*8);
	return( r );
}


// ÷÷÷÷÷÷÷÷÷÷÷   basic memory management   ÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷

/// deletes an array and sets pointer to zero
/** \param t pointer to array */
template <class t>
void dropa( t *& v )
{
	delete [] v;
	v = null;
}
/// deletes an instance and sets pointer to zero
/** \param t pointer to instance */
template <class t>
void dropp( t *& v )
{
	delete v;
	v = null;
}

/// copies memory
/** fast copy by using integers \param dst destination \param src source \param length number of bytes to copy */
inline void cpy( void *dst, const void *src, const int &length )
{
	#ifdef __OPTIMIZE__
	std::memmove( dst, src, length );
	#else
	uint *p, *pe, *ps;
	byte *b, *be, *bs;
	ps = (uint*)src - 1;
	p = (uint*)dst;
	pe = p + length / sizeof(uint);
	while( p < pe )
		*p++ = *++ps;
	bs = (byte*)( ps + 1 ) - 1;
	b = (byte*)p;
	be = (byte*)dst + length;
	while( b < be )
		*b++ = *++bs;
	#endif
}
template <typename t>
inline void cpyv( t &dst, t const& src )
{
	cpy( &dst, &src, sizeof(t) );
}
template <typename t, uint l>
inline void cpya( t (&dst)[l], t const (&src)[l] )
{
	cpy( dst, src, l*sizeof(t) );
}
/// copies memory reverse
inline void cpyr( void *dst, const void *src, const int &length )
{
	#ifdef __OPTIMIZE__
	std::memmove( dst, src, length );
	#else
	int i, l;
	uint *p, *pe, *ps;
	l = length - ( length % sizeof(uint) );
	for( i=length-1; i>l; i-- )
		*((byte*)dst+i) = *((byte*)src+i);
	i /= sizeof(uint);
	p = (uint*)dst + i;
	ps = (uint*)src + i + 1;
	pe = (uint*)src;
	while( ps > pe )
		*p-- = *--ps;
	#endif
}
/// zeros memory
/** fast setting to zero by use of integers \param dst destination \param length number of bytes to be set to zero */
inline void zro( void *dst, const int &length )
{
	#ifdef __OPTIMIZE__
	std::memset( dst, 0, length );
	#else
	uint *p, *pe;
	byte *b, *be;
	p = (uint*)dst;
	pe = p + length / sizeof(uint);
	while( p < pe )
		*p++ = 0;
	b = (byte*)p;
	be = (byte*)dst + length;
	while( b < be )
		*b++ = '\0';
	#endif
}
template <typename t, uint l>
inline void zro( t (&arr)[l] )
{
	zro( &arr, sizeof(arr) );
}
/// sets memory to 0xff
/** fast setting to 0xff by use of integers \param dst destination \param length number of bytes to be set to zero */
inline void ffs( void *dst, const int &length )
{
	#ifdef __OPTIMIZE__
	std::memset( dst, 0xff, length );
	#else
	uint *p, *pe;
	byte *b, *be;
	p = (uint*)dst;
	pe = p + length / sizeof(uint);
	while( p < pe )
		*p++ = 0xffffffff;
	b = (byte*)p;
	be = (byte*)dst + length;
	while( b < be )
		*b++ = '\xff';
	#endif
}
template <typename t, uint l>
inline void ffs( t (&arr)[l] )
{
	ffs( &arr, sizeof(arr) );
}
/// sets memory to byte value
/** fast setting to val by use of integers \param dst destination \param length number of bytes to be set to zero */
inline void set( void *dst, const int &length, const byte &val )
{
	uint *p, *pe, pv;
	byte *b, *be, bv;
	bv = val;
	pv = (uint)val << 24 | (uint)val << 16 | (uint)val << 8 | (uint)val;
	p = (uint*)dst;
	pe = p + length / sizeof(uint);
	while( p < pe )
		*p++ = pv;
	b = (byte*)p;
	be = (byte*)dst + length;
	while( b < be )
		*b++ = bv;
}
/// sets memory to integer value
/** fast setting to val by use of integers \param dst destination \param length number of bytes to be set to zero */
inline void set( void *dst, const int &length, const uint &val )
{
	uint *p, *pe, pv;
	byte *b, *be;
	pv = val;
	p = (uint*)dst;
	pe = p + length / sizeof(uint);
	while( p < pe )
		*p++ = pv;
	b = (byte*)p;
	be = (byte*)dst + length;
	while( b < be )
	{
		*b++ = (byte)pv;
		pv >>= 8;
	}
}
/// sets memory to short value
/** fast setting to val by use of integers \param dst destination \param length number of bytes to be set to zero */
inline void set( void *dst, const int &length, const ushort &val )
{
	uint *p, *pe, pv;
	byte *b, *be;
	pv = ( val << 16 ) | val;
	p = (uint*)dst;
	pe = p + length / sizeof(uint);
	while( p < pe )
		*p++ = pv;
	b = (byte*)p;
	be = (byte*)dst + length;
	while( b < be )
	{
		*b++ = (byte)pv;
		pv >>= 8;
	}
}
/// compares length bytes of two buffers
/** \return true if length bytes of buf1 and buf2 are equal */
inline bool d_memcmp( void const* buf1, void const* buf2, uint length )
{
	uint i = 0;
	if( length >= sizeof( uint ) )
	{
		uint const* b1 = (uint const*)buf1;
		uint const* b2 = (uint const*)buf2;
		uint j = length / sizeof( uint );
		while( i < j && b1[i] == b2[i] )
			i++;
		if( i != j )
			return( false );
		i *= sizeof( uint );
	}
	byte const* b1 = (byte const*)buf1;
	byte const* b2 = (byte const*)buf2;
	for( ; i < length && b1[i] == b2[i]; i++ );
	if( i != length )
		return( false );
	return( true );
}

template <typename t>
inline bool cmp( t const& v1, t const& v2 )
{
	return( d_memcmp( &v1, &v2, sizeof(t) ) );
}

template <typename t, uint l>
inline bool acmp( t const (&arr1)[l], t const (&arr2)[l] )
{
	return( d_memcmp( arr1, arr2, l ) );
}

template <typename t>
inline int epos( t const* arr, uint beg, uint end, t e )
{
	uint i;
	for( i = beg; i < end && arr[i] != e; i++ );
	if( i == end )
		return( -1 );
	return( i );
}

template <typename t>
inline int erpos( t const* arr, uint beg, uint end, t e )
{
	uint i;
	if( !end )
		return( -1 );
	for( i = end - 1; i > beg && arr[i] != e; i-- );
	if( i == beg && arr[i] != e )
		return( -1 );
	return( i );
}

template <typename t>
int apos( t const* arr, uint beg, uint end, t const* a, uint alen )
{
	if( !alen )
		return( 0 );
	if( end < alen )
		return( -1 );
	uint bsl = end + 1 - alen;
	t a0 = a[0];
	uint i;
	for( i = beg; i < bsl; i++ )
	{
		if( arr[i] == a0 )
		{
			uint k = 1;
			for( uint j = i + 1; k < alen && arr[j] == a[k]; j++, k++ );
			if( k == alen )
				return( i );
		}
	}
	return( -1 );
}

template<>
class type_t<double>
{
public:
	typedef ulint bm_t;
	// using bm_t = ulint;
protected:
	static_assert( sizeof( double ) == sizeof( bm_t ) );
	constexpr static uint bits_mant = 52;
	constexpr static uint bits_exp = type_t<bm_t>::numbits() - 1 - bits_mant;
	constexpr static bm_t u_mant = type_t<bm_t>::setbits( bits_mant );
	constexpr static bm_t u_nan  = (bm_t)1 << ( bits_mant - 1 );
	constexpr static bm_t u_exp2 = (bm_t)1 << bits_mant;
	constexpr static bm_t u_exp1 = type_t<bm_t>::setbits( bits_exp - 1 ) << ( bits_mant + 1 );
	constexpr static bm_t u_sign = (bm_t)1 << ( type_t<bm_t>::numbits() - 1 );
	/*
	constexpr static bm_t u_sign = 0x8000000000000000ull;
	constexpr static bm_t u_exp1 = 0x7fe0000000000000ull;
	constexpr static bm_t u_exp2 = 0x0010000000000000ull;
	constexpr static bm_t u_nan  = 0x0008000000000000ull;
	constexpr static bm_t u_mant = 0x000fffffffffffffull;
	*/
	/*
	constexpr static bm_t lmax  = 0x7fefffffffffffffull;			///< maximum valid value for a double in integer notation
	constexpr static bm_t lmin  = 0xffefffffffffffffull;			///< minimum valid value for a double in integer notation
	*/
public:
	constexpr static bool is_signed()
	{
		return( true );
	}
	constexpr static bool is_unsigned()
	{
		return( false );
	}
	constexpr static uint numbits()
	{
		return( sizeof( bm_t ) * 8 );
	}
	constexpr static bm_t setbits( uint num )
	{
		return( type_t<bm_t>::setbits( num ) );
	}
	static double max()
	{
		double d;
		bm_t u_max = bm_max();
		cpy( &d, &u_max, sizeof( d ) );
		return( d );
	}
	static double min()
	{
		double d;
		bm_t u_min = bm_min();
		cpy( &d, &u_min, sizeof( d ) );
		return( d );
	}
	static bm_t get_bm( double const& d )
	{
		bm_t l;
		cpy( &l, &d, sizeof( l ) );
		return( l );
	}
	static double set_bm( bm_t const& l )
	{
		double d;
		cpy( &d, &l, sizeof( l ) );
		return( l );
	}
	constexpr static uint numbits_mant()
	{
		return( bits_mant );
	}
	constexpr static uint numbits_exp()
	{
		return( bits_exp );
	}
	constexpr static bm_t bm_max()
	{
		return( u_exp1 | u_mant );
	}
	constexpr static bm_t bm_min()
	{
		return( u_sign | u_exp1 | u_mant );
	}
	constexpr static bm_t bm_exp()
	{
		return( u_exp1 | u_exp2 );
	}
	constexpr static bm_t bm_mant()
	{
		return( u_mant );
	}
	constexpr static bm_t bm_sign()
	{
		return( u_sign );
	}
	constexpr static bm_t bm_nan()
	{
		return( u_nan );
	}
};

template<>
class type_t<float>
{
public:
	typedef uint bm_t;
	// using bm_t = ulint;
protected:
	static_assert( sizeof( float ) == sizeof( bm_t ) );
	constexpr static uint bits_mant = 23;
	constexpr static uint bits_exp = type_t<bm_t>::numbits() - 1 - bits_mant;
	constexpr static bm_t u_mant = type_t<bm_t>::setbits( bits_mant );
	constexpr static bm_t u_nan  = (bm_t)1 << ( bits_mant - 1 );
	constexpr static bm_t u_exp2 = (bm_t)1 << bits_mant;
	constexpr static bm_t u_exp1 = type_t<bm_t>::setbits( bits_exp - 1 ) << ( bits_mant + 1 );
	constexpr static bm_t u_sign = (bm_t)1 << ( type_t<bm_t>::numbits() - 1 );
public:
	constexpr static bool is_signed()
	{
		return( true );
	}
	constexpr static bool is_unsigned()
	{
		return( false );
	}
	constexpr static uint numbits()
	{
		return( sizeof( bm_t ) * 8 );
	}
	constexpr static bm_t setbits( uint num )
	{
		return( type_t<bm_t>::setbits( num ) );
	}
	static float max()
	{
		float d;
		bm_t u_max = bm_max();
		cpy( &d, &u_max, sizeof( d ) );
		return( d );
	}
	static float min()
	{
		float d;
		bm_t u_min = bm_min();
		cpy( &d, &u_min, sizeof( d ) );
		return( d );
	}
	static bm_t get_bm( float const& d )
	{
		bm_t l;
		cpy( &l, &d, sizeof( l ) );
		return( l );
	}
	static float set_bm( bm_t const& l )
	{
		double d;
		cpy( &d, &l, sizeof( l ) );
		return( l );
	}
	constexpr static uint numbits_mant()
	{
		return( bits_mant );
	}
	constexpr static uint numbits_exp()
	{
		return( bits_exp );
	}
	constexpr static bm_t bm_max()
	{
		return( u_exp1 | u_mant );
	}
	constexpr static bm_t bm_min()
	{
		return( u_sign | u_exp1 | u_mant );
	}
	constexpr static bm_t bm_exp()
	{
		return( u_exp1 | u_exp2 );
	}
	constexpr static bm_t bm_mant()
	{
		return( u_mant );
	}
	constexpr static bm_t bm_sign()
	{
		return( u_sign );
	}
	constexpr static bm_t bm_nan()
	{
		return( u_nan );
	}
};

typedef type_t<double> type_double;


// ÷÷÷÷÷÷÷÷÷÷÷   complex memory management   ÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷

/// stream class for dynamically allocating memory
template <class T> class stream
{
protected:
	T *data;	///< buffer
	static const uint s = sizeof( T );		///< size of an element
	uint l;		///< length of buffer
	uint cl;	///< chunk length
	uint p;		///< number of elements
public:
	/// constructor, determines chunk size
	/** chunk size is quantisized to size of class T \param _cs chunk size in bytes \return instance \n if _cs is zero, initial chunk size is one element and multiplied by two each reallocation */
	stream( uint const& _cs = 0 )
	{
		data = null;
        if( _cs )
			cl = ( _cs - 1 ) / s + 1;
		else
			cl = 0;
		p = 0;
		l = 0;
	}
	/// copy constructor
	stream( stream const& _stream )
	{
		data = null;
        cl = 0;
		p = 0;
		l = 0;
		*this = _stream;
	}
	/// assignment operator
	stream& operator =( stream const& _stream )
	{
		uint i, j, n;
		drop();
        cl = _stream.cl;
        n = _stream.num();
		if( n > l )
        {
        	delete [] data;
            if( cl )
	            l = alloc( data, ((n-1)/cl+1)*cl );
            else
            	if( l )
	            	l = alloc( data, ((n-1)/l+1)*l );
                else
                    if( n )
                    {
						for( i=n-1, j=0; i; j++ )
                            i>>=1;
                		l = alloc( data, 1<<j );
                    } else {
                    	l = 0;
                        data = null;
	                }
        }
		for( i=0; i<n; i++ )
			add( *_stream.get( i ) );
		return( *this );
	}
	/// move constructor
	stream( stream && _stream )
	{
        l = _stream.l;
        cl = _stream.cl;
        p = _stream.p;
        data = _stream.data;
        _stream.data = null;
        _stream.l = null;
        _stream.p = null;
	}
	/// move assignment operator
	stream& operator =( stream && _stream )
	{
		free();
        l = _stream.l;
        cl = _stream.cl;
        p = _stream.p;
        data = _stream.data;
        _stream.data = null;
        _stream.l = null;
        _stream.p = null;
		return( *this );
	}
	/// destructor
	~stream()
	{
        if( data )
		    delete [] data;
	}
	/// returns an element
	/** \param _p position in number of elements \return value of element _p */
	inline T const& getval( uint const& _p ) const
	{
		return( *( data + _p ) );
	}
	/// returns pointer to an element
	/** \param _p position in number of elements \return pointer to element _p */
	inline T * get( uint const& _p ) const
	{
		return( data + _p );
	}
	/// returns pointer to first element
	/** returns zero if element is not available */
	inline T* getfirst() const
	{
		if( p )
			return( data );
		else
			return( null );
	}
	/// returns pointer to last element
	/** returns zero if element is not available */
	inline T* getlast() const
	{
    	if( p )
			return( data + p - 1 );
        else
        	return( null );
	}
	/*
	no idea why this should be necessary
	/// adds one element to the end
	** copies element d \param d new element \return number of elements in stream or zero if failed *
	inline uint add( T &d )
	{
		if( p >= l )
			expand();
		if( p < l )
		{
			data[p++] = d;
			return( p );
		} else
			return( 0 );
	}
	*/
	/// adds one element to the end
	/** copies element d \param d element to be added \return number of elements in stream or zero if failed */
	inline uint add( T const& d )
	{
		if( p >= l )
			expand();
		if( p < l )
		{
			data[p++] = d;
			return( p );
		} else
			return( 0 );
	}
	/// adds one element several times to the end
	/** copies element d \param d new element to be duplicated \return number of elements in stream */
	inline uint add_copies( T const& d, uint num )
	{
		uint i, n, s;

        n = p + num;
        if( n > l )
        {
            if( cl )
                s = ((n-1)/cl+1)*cl;
			else {
				for( i=n-1, s=0; i>0; i>>=1, s++ );
				s = 1 << s;
			}
			expand( s - l );
		}
		for( i=0; i<num && p < l; i++ )
			data[p++] = d;
		return( p );
	}
	/// adds several elements to the end
	/** copies element d \param *d pointer to array of elements \return number of elements in stream */
	inline uint add( T const* d, uint num )
	{
		uint i, n, s;

		n = p + num;
		if( n > l )
		{
			if( cl )
				s = ((n-1)/cl+1)*cl;
			else {
				for( i=n-1, s=0; i>0; i>>=1, s++ );
				s = 1 << s;
			}
			expand( s - l );
		}
		for( i=0; i<num && p < l; i++ )
			data[p++] = d[i];
		return( p );
	}
	/// replaces one element
	/** copies element d \param pointer to new element \param _p position in number of elements \return number of elements in stream or zero if failed */
	inline uint set( T const& d, uint const& _p )
	{
		if( _p < p )
		{
			data[_p] = d;
			return( p );
		} else {
			return( 0 );
		}
	}
	/// inserts one element
	/** copies element d \param pointer to new element \param _p position in number of elements where to insert d \return number of elements in stream or zero if failed */
	inline uint ins( T const& d, uint _p )
	{
		uint i;
		if( p >= l )
			expand();
		if( p < l )
		{
			if( _p > p )
				_p = p;
			for( i=p; i>_p; i-- )
				data[i] = data[i-1];
			p++;
			data[_p] = d;
			return( p );
		} else
			return( 0 );
	}
	/// deletes one element
	/** \param _p position in number of elements of element to delete d \return number of elements in stream or zero if failed */
	inline uint del( uint const& _p )
	{
		uint i;
		if( p > 0 && _p < p )
		{
			p--;
			for( i=_p; i<p; i++ )
				data[i] = data[i+1];
			return( p );
		} else {
			return( 0 );
		}
	}
	/// deletes multiple elements
	/** \param _p position in number of elements of element to delete \param _num number of elements to delete d \return number of elements in stream or zero if failed */
	inline uint del( uint const& _p, uint const& _num )
	{
		uint i;
		if( p > 0 && _p+_num-1 < p )
		{
			p-=_num;
			for( i=_p; i<p; i++ )
				data[i] = data[i+_num];
			return( p );
		} else {
			return( 0 );
		}
	}
	/// truncates buffer
	/** reallocates buffer to hold exactly p number of elements */
	inline void trunc()
	{
		uint i;
		T *d;
		if( p )
		{
			if( ( i = alloc( d, p ) ) )
			{
				l = i;
				for( i=0; i<p; i++ )
					d[i] = data[i];
				delete [] data;
				data = d;
				d = null;
			}
		} else {
			free();
		}
	}
	/// deletes all elements and frees buffer
	inline void free()
	{
		drop();
		if( data )
		{
			l = 0;
			delete [] data;
			data = null;
		}
	}
	/// invalidates all elements
	inline void drop()
	{
		p = 0;
	}
	/// returns pointer to buffer
	inline T* getdata() const
	{
		return( data );
	}
	/// sets chunk length in number of elements
	inline void setcl( int _cl )
	{
		if( _cl > 0 )
			cl = _cl;
	}
	/// gets number of elements
	/** \return number of elements in stream */
	inline uint const& num() const
	{
		return( p );
	}
	/// gets allocated length
	/** \return allocated length in number of elements */
	inline uint const& alength() const // allocated size
	{
		return( l );
	}
	/// gets allocated size
	/** \return allocated size in bytes */
	inline uint asize() const // allocated size
	{
		return( l*s );
	}
	/// gets used size
	/** \return used size in bytes */
	inline uint usize() const // used size
	{
		return( p*s );
	}
	/// gets chunk size
	/** \return chunk size in bytes */
	inline uint csize() const // chunk size
	{
		return( cl*s );
	}
	/// gets element size
	/** \return size of one element in bytes */
	inline uint const& ssize() const // element size
	{
		return( s );
	}
	/// gets sum of elememts
	/** \return sum of all elements */
	inline T const sum()
	{
		int i;
		T t;
		t = 0;
		for( i=0; i<p; i++ )
			t += data[i];
		return( t );
	}
protected:
	/** allocates new memory block with size of old memory block plus chunk size, copies all data to new memory block, deletes old memory block */
	inline void expand( int n = 1 )
	{
		uint i;
		T *d;

		if( cl )
			i = alloc( d, l+cl*n );
		else
			if( n > 1 )
				i = alloc( d, l+n );
			else
				if( l )
					i = alloc( d, l<<1 );
				else
					i = alloc( d, 1 );
		if( i )
		{
			l = i;
			for( i=0; i<p; i++ )
				d[i] = data[i];
			delete [] data;
			data = d;
			d = null;
		}
	}
	/** allocates new memory with given size in chunk size */
	inline uint alloc( T *& d, uint l )
	{
		if( l > 0 )
		{
        	if( !( d = new (std::nothrow) T[l] ) )
                l = 0;
		} else {
			d = null;
			l = 0;
		}
		return( l );
	}
};

/// encapsulates stream <T*> to handle pointers to arrays of elements and its contents. instance cannot be duplicated!
/** class is suitable for storing large amount of data. reallocation of stream <T*> only copies pointers, no contents. contents are deleted upon class destruction, therefore copying instance is prohibited */
template <class T> class astream
{
protected:
	stream <T*> data;
	T* nul;
	/// not allowed: copy constructor
	astream( const astream &_stream ) { }
	/// not allowed: assignment operator
	astream & operator = ( const astream &_stream ) { return( *this ); }
public:
	astream( const uint & _cs = 0 )
	{
		data = stream <T*>( _cs );
		nul = null;
	}
	~astream()
	{
		drop();
	}
	inline T* get( const uint & _p ) const
	{
		if( _p < data.num() )
			return( *data.get( _p ) );
		else
			return( null );
	}
	inline T* getfirst() const
	{
    	if( data.num() )
			return( *data.getfirst() );
        else
			return( nul );
	}
	inline T* getlast() const
	{
    	if( data.num() )
			return( *data.getlast() );
        else
        	return( nul );
	}
	inline uint add( T *d )
	{
		return( data.add( d ) );
	}
	inline uint add( const T **d, int num )
	{
		int i;

		for( i=0; i<num; i++ )
			data.add( d+i );
		return( data.num() );
	}
	inline uint set( const T &d, const uint & _p )
	{
		return( data.set( d, _p ) );
	}
	inline uint del( const uint & _p )
	{
		if( _p < data.num() )
		{
			delete [] *data.get( _p );
			return( data.del( _p ) );
		} else {
			return( 0 );
		}
	}
	inline uint del( const uint & _p, const uint & _num )
	{
		uint i;
		if( _p+_num-1 < data.num() )
		{
			for( i=0; i<_num; i++ )
				delete [] *data.get( i+_p );
			return( data.del( _p, _num ) );
		} else {
			return( 0 );
		}
	}
	inline void trunc()
	{
		data.trunc();
	}
	inline void free()
    {
		drop();
    }
	inline void drop()
	{
		uint i;
		for( i=0; i<data.num(); i++ )
			delete [] *data.get( i );
		data.drop();
	}
	inline stream <T*> const getdata() const
	{
		return( data );
	}
	inline void setcl( int _cl )
	{
		data.setcl( _cl );
	}
	inline uint const& num() const
	{
		return( data.num() );
	}
	inline uint const& alength() const // allocated size
	{
		return( data.alength() );
	}
	inline uint const& clength() const // chunk length
	{
		return( data.clength() );
	}
	inline uint asize() const // allocated size
	{
		return( data.asize() );
	}
	inline uint usize() const // used size
	{
		return( data.usize() );
	}
	inline uint csize() const // chunk size
	{
		return( data.csize() );
	}
	inline uint const& ssize() const // element size
	{
		return( data.ssize() );
	}
};

/// encapsulates stream <T*> to handle pointers to elements and its contents. instance cannot be duplicated!
/** class is suitable for storing large amount of data. reallocation of stream <T*> only copies pointers, no contents. contents are deleted upon class destruction, therefore copying instance is prohibited */
template <class T> class pstream
{
protected:
	stream <T*> data;
	static constexpr T* nul = null;
	/// not allowed: copy constructor
	pstream( const pstream &_stream ) {}
	/// not allowed: assignment operator
	pstream & operator =( const pstream &_stream ) { return( *this ); }
public:
	pstream()
	{
		// nul = null;
	}
	~pstream()
	{
		drop();
	}
	inline T* get( const uint & _p ) const
	{
		return( *data.get( _p ) );
	}
	inline T* getfirst() const
	{
		if( data.num() )
			return( *data.getfirst() );
		else
			return( nul );
	}
	inline T* getlast() const
	{
		if( data.num() )
	        return( *data.getlast() );
		else
			return( nul );
	}
	inline bool all_valid() const
	{
		bool okay = true;
		for( uint i=0; i<data.num(); i++ )
			okay &= *data.get( i );
		return( okay );
	}
	inline bool add( T *&& d )
	{
		bool okay = data.num() + 1 == data.add( d ) && d;
		d = null;
		return( okay );
	}
	inline bool add( T *& d )
	{
		bool okay = data.num() + 1 == data.add( d ) && d;
		d = null;
		return( okay );
	}
	inline bool set( T *&& d, const uint & _p )
	{
		if( _p < data.num() )
		{
            dropp( *data.get( _p ) );
            bool okay = d;
            data.set( d, _p );
            d = null;
			return( okay );
		} else {
			return( false );
		}
	}
	inline bool set( T *& d, const uint & _p )
	{
		if( _p < data.num() )
		{
            dropp( *data.get( _p ) );
            bool okay = d;
            data.set( d, _p );
            d = null;
			return( okay );
		} else {
			return( false );
		}
	}
	inline bool ins( T *&& d, uint _p )
	{
		bool okay = data.num() + 1 == data.ins( d, _p ) && d;
		d = null;
        return( okay );
	}
	inline bool ins( T *& d, uint _p )
	{
		bool okay = data.num() + 1 == data.ins( d, _p ) && d;
		d = null;
        return( okay );
	}
	inline T *forfeit( const uint & _p )
	{
		T *t;
		
		t = null;
		if( _p < data.num() )
        {
        	t = *data.get( _p );
			*data.get( _p ) = 0;
		}
		return( t );
	}
	inline bool del( const uint & _p )
	{
		if( _p < data.num() )
		{
            dropp( *data.get( _p ) );
            data.del( _p );
			return( true );
		} else {
			return( false );
		}
	}
	inline bool del( const uint & _p, const uint & _num )
	{
		uint i, p;
		if( data.num() > 0 && _p+_num-1 < data.num() )
		{
			p = _p + _num;
			for( i=_p; i<p; i++ )
                dropp( *data.get( i ) );
			data.del( _p, _num );
			return( true );
		} else {
			return( false );
		}
	}
	inline void trunc()
	{
        data.trunc();
	}
	inline void free()
    {
    	drop();
        data.free();
    }
	inline void drop()
	{
        uint i;
        for( i=0; i<data.num(); i++ )
            dropp( *data.get( i ) );
        data.drop();
	}
	inline uint const& num() const
	{
		return( data.num() );
	}
	inline uint const& alength() const // allocated size
	{
		return( data.alength() );
	}
	inline uint const& clength() const // chunk length
	{
		return( data.clength() );
	}
	inline uint asize() const // allocated size
	{
		return( data.asize() );
	}
	inline uint usize() const // used size
	{
		return( data.usize() );
	}
	inline uint csize() const // chunk size
	{
		return( data.csize() );
	}
	inline uint const& ssize() const // element size
	{
		return( data.ssize() );
	}
	inline void setcl( int _cl )
	{
		data.setcl( _cl );
	}
};



// ÷÷÷÷÷÷÷÷÷÷÷   sorting algorithms   ÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷
/*
/// returns 1 if buf1 > buf2, -1 if buf1 < buf2, 0 if buf1 == buf2
template <typename T> inline int d_strweigh( const T *buf1, const T *buf2 )
{
	int weight;
	if( buf1 )
	{
		if( buf2 )
		{
			--buf1;
			--buf2;
			while( *++buf1 == *++buf2 && *buf1 && *buf2 );
			if( *buf1 > *buf2 )
				weight = 1;
			else
				if( *buf1 < *buf2 )
					weight = -1;
				else
					weight = 0;
		} else {
			weight = 1;
		}
	} else {
		if( buf2 )
			weight = -1;
		else
			weight = 0;
	}
	return( weight );
}


inline int d_wstrcmp_intret( const wchar *buf1, const wchar *buf2, int length, int &cmplength )
{
	int j, k, l;
	for( cmplength=0, j=0, k=-1, l=-1; ( length < 0 || cmplength < length ) && !j && k && l; cmplength++ )
	{
		k = buf1[cmplength];
		l = buf2[cmplength];
		j = k - l;
	}
	return( j );
}
*/

/// slow stable sort \n reverse sort, unchanged source, keeps order
/** \param tosort array to be sorted \param sortnumbers positions of numbers in tosort will be stored here \param tosortlength length of tosort and sortnumbers */
inline void asortr( const lint *tosort, int *sortnumbers, int tosortlength )
{
	int lastpos, lasthipos, i, j;
	lint lasthi, last;
	lasthi = type_t<lint>::max();
	lasthipos = tosortlength;
	for( j=0; j<tosortlength; j++ )
	{
		last = type_t<lint>::min();
		lastpos = 0;
		for( i=0; i<tosortlength; i++ )
		{
			if( tosort[i]>last && ( tosort[i]<lasthi || ( tosort[i]==lasthi && i>lasthipos ) ) )
			{
				lastpos = i;
				last = tosort[i];
			}
		}
		sortnumbers[j] = lastpos;
		lasthipos = lastpos;
		lasthi = last;
	}
}
/// slow stable sort \n unchanged source, keeps order
inline void asort( const lint *tosort, int *sortnumbers, int tosortlength )
{
	int i, j, lastpos, lastjpos;
	lint lastval, lastjval;
	lastjval = type_t<lint>::min();
	lastjpos = tosortlength;
	for( j=0; j<tosortlength; j++ )
	{
		lastval = type_t<lint>::max();
		for( i=0; i<tosortlength; i++ )
		{
			if( tosort[i] < lastval && ( tosort[i] > lastjval || ( tosort[i] == lastjval && i > lastjpos ) ) )
			{
				lastval = tosort[i];
				lastpos = i;
			}
		}
		sortnumbers[j] = lastpos;
		lastjpos = lastpos;
		lastjval = lastval;
	}
}
/// slow stable sort \n reverse sort, unchanged source, keeps order
inline void asortr( const double *tosort, int *sortnumbers, int tosortlength )
{
	int lastpos, lasthipos, i, j;
	double lasthi, last;
	lasthi = type_double::max();
	lasthipos = tosortlength;
	for( j=0; j<tosortlength; j++ )
	{
		last = type_double::min();
		lastpos = 0;
		for( i=0; i<tosortlength; i++ )
		{
			if( tosort[i]>last && ( tosort[i]<lasthi || ( tosort[i]==lasthi && i>lasthipos ) ) )
			{
				lastpos = i;
				last = tosort[i];
			}
		}
		sortnumbers[j] = lastpos;
		lasthipos = lastpos;
		lasthi = last;
	}
}
/// slow stable sort \n unchanged source, keeps order
inline void asort( const double *tosort, int *sortnumbers, int tosortlength )
{
	int i, j, lastpos, lastjpos;
	double lastval, lastjval;
	lastjval = type_double::min();
	lastjpos = tosortlength;
	for( j=0; j<tosortlength; j++ )
	{
		lastval = type_double::max();
		for( i=0; i<tosortlength; i++ )
		{
			if( tosort[i] < lastval && ( tosort[i] > lastjval || ( tosort[i] == lastjval && i > lastjpos ) ) )
			{
				lastval = tosort[i];
				lastpos = i;
			}
		}
		sortnumbers[j] = lastpos;
		lastjpos = lastpos;
		lastjval = lastval;
	}
}
/// slow stable sort \n reverse sort, unchanged source, keeps order
inline void asortr( const int *tosort, int *sortnumbers, int tosortlength )
{
	int lastpos, lasthipos, i, j;
	int lasthi, last;
	lasthi = type_t<int>::max();
	lasthipos = tosortlength;
	for( j=0; j<tosortlength; j++ )
	{
		last = type_t<int>::min();
		lastpos = 0;
		for( i=0; i<tosortlength; i++ )
		{
			if( tosort[i]>last && ( tosort[i]<lasthi || ( tosort[i]==lasthi && i>lasthipos ) ) )
			{
				lastpos = i;
				last = tosort[i];
			}
		}
		sortnumbers[j] = lastpos;
		lasthipos = lastpos;
		lasthi = last;
	}
}
/// slow stable sort \n unchanged source, keeps order
inline void asort( const int *tosort, int *sortnumbers, int tosortlength )
{
	int i, j, lastpos, lastjpos;
	int lastval, lastjval;
	lastjval = type_t<int>::min();
	lastjpos = tosortlength;
	for( j=0; j<tosortlength; j++ )
	{
		lastval = type_t<int>::max();
		for( i=0; i<tosortlength; i++ )
		{
			if( tosort[i] < lastval && ( tosort[i] > lastjval || ( tosort[i] == lastjval && i > lastjpos ) ) )
			{
				lastval = tosort[i];
				lastpos = i;
			}
		}
		sortnumbers[j] = lastpos;
		lastjpos = lastpos;
		lastjval = lastval;
	}
}
/// faster sort \n changes source, doesn't keep order
/** resembles selection sort */
template <typename t>
inline void bsort( t * tosort, uint * sortnumbers, uint tosortlength )
{
	t lastval, val;
	uint lastpos = 0;

	for( uint i = 0; i < tosortlength; i++ )
		sortnumbers[i] = i;

	for( uint j = 0; j < tosortlength; j++ )
	{
		uint i;
		lastval = type_t<t>::max();
		for( i = j; i < tosortlength; i++ )
		{
			val = tosort[i];
			if( val < lastval )
			{
				lastval = val;
				lastpos = i;
			}
		}
		tosort[lastpos] = tosort[j];
		tosort[j] = lastval;
		i = sortnumbers[lastpos];
		sortnumbers[lastpos] = sortnumbers[j];
		sortnumbers[j] = i;
	}
}

/// faster sort \n changes source, doesn't keep order
/** resembles selection sort */
/*
inline void bsort( wchar **tosort, int *sortnumbers, int tosortlength )
{
	wchar *lastval, *val;
	int i, j, lastpos, cmplength;

	for( i=0; i<tosortlength; i++ )
		sortnumbers[i] = i;

	for( j=0; j<tosortlength; j++ )
	{
		lastval = null;
		for( i=j; i<tosortlength; i++ )
		{
			val = tosort[i];
			if( !lastval || d_wstrcmp_intret( val, lastval, -1, cmplength ) < 0 )
			{
				lastval = val;
				lastpos = i;
			}
		}
		tosort[lastpos] = tosort[j];
		tosort[j] = lastval;
		i = sortnumbers[lastpos];
		sortnumbers[lastpos] = sortnumbers[j];
		sortnumbers[j] = i;
	}
}
*/


#endif // DB_MEM_INCLUDED
