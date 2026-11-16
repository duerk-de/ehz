__GIT_SYMLINKS__
target=../_inc/db_str.cpp
target_resolved=/dibom/src/c/_inc/db_str.cpp
kind=file
sha256=e98ffc5c59f55cffe88507770a14cd716c1273c685a60e06d81396fb67e2ce02
size=36103
created=2026-11-16T20:49:12.755047
--END-HEADER--
#ifndef DB_STR_INCLUDED
#define DB_STR_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_pointclass.cpp"
#include "db_rectclass.cpp"

#include <cstdarg>

// ÷÷÷÷÷÷÷÷÷÷÷   string formatting algorithms   ÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷÷


/// copies memory, adds zero byte to the end
/** fast copy by using cpy. dst has to be allocated to at least length+1 bytes. \param dst destination \param src source \param length number of bytes to copy */
/*
inline int cscpy( void *dst, const void *src, const int &length )
{
	if( length > -1 )
	{
		cpy( dst, src, length );
		*( (byte*)dst + length ) = '\0';
		return( length );
	} else {
		return( 0 );
	}
}
*/
/// copies memory, adds zero to the end
/** dst has to be allocated to at least length+1 elements. \param dst destination \param src source \param length number of elements to copy */
/*
template <typename T> inline int scpy( T *dst, const T *src, const int &length )
{
	cpy( dst, src, length );
	dst[length] = 0;
	return( d_max( 0, length ) );
}
*/


/// searches last occurence of a string within another string
/** \param buf string buffer \param buflen length of buf \param str string to search for \param strlen length of str \return <0 if str not found, position of str in buf otherwise */
template <typename T> inline int d_strrpos( const T *buf, int buflen, const T *str, int strlen )
{
	int i, j, k, nfound, strlen1, strlen2;
	char beg;

	nfound = true;
	strlen1 = strlen - 1;
	strlen2 = strlen1 - 1;
	beg = str[strlen1];
	for( i=buflen-1; i>strlen2 && nfound; i-- )
	{
		if( buf[i] == beg )
		{
			for( j=strlen2, k=i-1; j>-1 && buf[k] == str[j]; j--, k-- );
			if( j == -1 )
				nfound = false;
		}
	}
	if( nfound )
		i = -1;
	else
		i++;
	return( i );
}
/// returns length of string bounded to maxlength
/** \param string null terminated string \param maxlength maximum valid length (if null termination is not found) \return length of string (bounded if equal to maxlength) */
uint d_strlen( char const* string, uint maxlength = type_t<uint>::max() )
{
	uint i;
	for( i = 0; i < maxlength && string[i]; i++ );
	return( i );
}
/// returns length of string bounded to maxlength
/** \param string null terminated string \param maxlength maximum valid length (if null termination is not found) \return length of string (bounded if equal to maxlength) */
template <uint len>
uint slen( char const (&str)[len] )
{
	uint i;
	for( i = 0; i < len && str[i]; i++ );
	return( i );
}

uint slen( char const *str, uint len )
{
	uint i;
	for( i = 0; i < len && str[i]; i++ );
	return( i );
}

struct cstr_t
{
	char const* ptr;
	uint len;
	
	constexpr cstr_t() : 
		ptr( nullptr ),
		len( 0 )
	{ }
	
	constexpr cstr_t( char const* _str, uint _len ) :
		ptr( _str ),
		len( _len )
	{ }
	
	template <uint _len>
	constexpr cstr_t( char const (&_str)[_len] ) :
		ptr( _str ),
		len( _len )
	{ }
	
	uint slen() const
	{
		return( d_min( len, ::slen( ptr, len ) ) );
	}
	
	bool terminated() const
	{
		return( slen() < len );
	}
	
	bool operator ==( cstr_t const& s ) const
	{
		uint l = d_min( len, s.len );
		for( uint i = 0; i < l; i++ )
		{
			if( ptr[i] == '\0' && s.ptr[i] == '\0' )
				return( true );
			if( ptr[i] != s.ptr[i] )
				return( false );
		}
		return( false );
	}
};

/// allocates and returns buffer holding a copy of string
/** \param string string to copy \param length [out] length of string and buffer \return pointer to buffer */
/*
template <typename T> inline char *makecbuf( const T *string, int &length )
{
	int i;
	char *buf;
	buf = null;
	if( string )
	{
		length = d_strlen( string );
		if( buf = new (std::nothrow) char[length+1] )
		{
			i = 0;
			while( i<length )
				buf[i++] = string[i];
			buf[i] = 0;
		} else {
			length = 0;
		}
	}
	return( buf );
}
*/
/// allocates and returns buffer holding a copy of string
/** \param string string to copy \return pointer to buffer */
/*
template <typename T> inline char *makecbuf( const T *string )
{
	int l;
	return( makecbuf( string, l ) );
}
*/

/*
bool d_substr( int &beg, int &end,
               const char *str, int begpos, int endpos,
               const char *begstr = null, int begstrlen = 0,
               const char *endstr = null, int endstrlen = 0,
               bool strict = false )
{
	beg = begpos;
	if( begstr != null )
	{
		if( !begstrlen )
			begstrlen = (int)d_strlen( begstr );
		beg = d_strpos( str, beg, endpos, begstr, begstrlen );
		if( beg < 0 )
		{
			beg = endpos;
			end = beg;
			return( false );
		}
		beg += begstrlen;
	}
	if( endstr != null )
	{
		if( !endstrlen )
			endstrlen = (int)d_strlen( endstr );
		end = d_strpos( str, beg, endpos, endstr, endstrlen );
		if( end < 0 )
		{
			end = endpos;
			if( strict )
				return( false );
		}
	} else {
		end = endpos;
	}
	return( true );
}
*/

/// copies a string to a string buffer
/** \param buf string buffer \param str string \return number of elements added to string buffer (excluding terminating null character) */
inline uint str2strn( char * buf, uint buflen, char const* str )
{
	uint i;
	for( i = 0; i < buflen && ( buf[i] = str[i] ); i++ );
	if( i == buflen && i != 0 && buf[i-1] )
		buf[--i] = '\0'; // error, did not fit
	return( i );
}
template <uint l>
inline uint str2strn( char * buf, uint buflen, char const (&str)[l] )
{
	uint i;
	for( i = 0; i < buflen && ( buf[i] = str[i] ); i++ );
	if( i == buflen && i != 0 && buf[i-1] )
		buf[--i] = '\0'; // error, did not fit
	return( i );
}
/// converts character to lowercase
inline char tolower( char const& c )
{
	if( c > 0x40 && c < 0x5b )
		return( c + 0x20 );
	else
		return( c );
}
/// converts character to uppercase
inline char toupper( char const& c )
{
	if( c > 0x60 && c < 0x7b )
		return( c - 0x20 );
	else
		return( c );
}
/*
/// converts characters of a string to lowercase
uint strtolower( char const* str )
{
	uint i = 0;
	--str;
	while( *++str )
	{
		i++;
		if( *str > 0x40 && *str < 0x5b )
			*str += 0x20;
	}
	return( i );
}
/// converts characters of a string to uppercase
uint strtoupper( char const* str )
{
	uint i = 0;
	--str;
	while( *++str )
	{
		i++;
		if( *str > 0x60 && *str < 0x7b )
			*str -= 0x20;
	}
	return( i );
}

/// converts characters of a string to uppercase
uint strreplace( char *str, char const& source, char const& replacewidth )
{
	uint i = 0;
	--str;
	while( *++str )
	{
		i++;
		if( *str == source )
			*str = replacewidth;
	}
	return( i );
}
*/

/// pads space characters before a substring to achieve a minumum length
/** \param buf position from where a substring was added \param length length of these substring \param minimum length of substring \return total length of substring */
uint strpad( char * buf, uint length, uint minlength, char padchar = ' ' )
{
	if( minlength > length )
	{
		uint l = minlength - length;
		buf--;
		uint k;
		for( k = minlength; k > l; k-- )
			buf[k] = buf[k-l];
		for( ; k > 0; k-- )
			buf[k] = padchar;
		buf++;
		buf[minlength] = 0;
		return( minlength );
	} else {
		return( length );
	}
}

/// converts integer numbers to a string
/** \param buf string buffer \param num number to be formatted and written to buf \param radix radix to use \param padto \param minimum length of number (zeros are added to front)
	\param minlength minimum length of number (spaced are added to front) \return number of characters added to buf excluding terminating null character */
template <typename num_t>
inline uint i2strn( char *buf, uint buflen,
                    num_t num,
                    uint spacefilllen = 0,
                    uint zerofilllen = 0,
                    uint radix = 10,
                    uint pointat = -1 )
{
	uint bufpos = 0;
	bool negsign = false;
	if( type_t<num_t>::is_signed() && num < 0 )
	{
		if( num == type_t<num_t>::min() )
			num = 0;
		else
			num = - num;
		negsign = true;
	}
	if( !num && bufpos < buflen )
	{
		if( pointat == bufpos && bufpos + 1 < buflen )
			buf[bufpos++] = '.';
		buf[bufpos++] = '0';
	} else {
		while( num && bufpos < buflen )
		{
			uint digit = num % (num_t)radix;
			if( digit < 10 )
				digit += '0';
			else
				digit += 'a' - 10;
			if( pointat == bufpos && bufpos + 1 < buflen )
				buf[bufpos++] = '.';
			buf[bufpos++] = (char)digit;
			num = num / (num_t)radix;
		}
	}
	if( zerofilllen > buflen )
		zerofilllen = buflen;
	if( negsign && zerofilllen )
		zerofilllen--;
	while( bufpos < zerofilllen )
	{
		if( pointat == bufpos && bufpos + 1 < buflen )
			buf[bufpos++] = '.';
		buf[bufpos++] = '0';
	}
	if( negsign && bufpos < buflen )
		buf[bufpos++] = '-';
	if( bufpos )
	{
		// reverse the buffer since we wrote the number backwards
		uint c = bufpos >> 1;
		uint s = 0;
		uint t = bufpos - 1;
		while( s < c )
		{
			char k = buf[s];
			buf[s++] = buf[t];
			buf[t--] = k;
		}
	}
	if( bufpos == buflen )
	{
		if( bufpos )
			buf[--bufpos] = '\0';
	} else {
		buf[bufpos] = '\0';
		if( buflen )
		{
			if( spacefilllen >= buflen )
				spacefilllen = buflen - 1;
			if( spacefilllen > bufpos )
				bufpos = strpad( buf, bufpos, spacefilllen );
		}
	}
	return( bufpos );
}

/// converts string to integer
template <typename I = int>
inline I str2int( char const* buf, uint const& beg, uint const& end,
                  char radix, uint &numparsed )
{
	I n = 0;
	bool s = false;
	uint b = beg;
	uint e = end > b + 66 ? b + 66 : end;
	if( b < e )
	{
		if( buf[b] == '-' )
		{
			s = true;
			b++;
		} else {
			if( buf[b] == '+' )
				b++;
		}
	}
	if( radix > 10 )
	{
		while( b < e )
		{
			char c = buf[b];
			if( c >= '0' && c <= '9' )
				n = n * (I)radix + (I)( c - '0' );
			else if( c >= 'A' && c < 'A' + radix - 10 )
				n = n * (I)radix + (I)( c - 'A' ) + 10;
			else if( c >= 'a' && c < 'a' + radix - 10 )
				n = n * (I)radix + (I)( c - 'a') + 10;
			else
				break;
			b++;
		}
	} else {
		while( b < e )
		{
			char c = buf[b];
			if( c >= '0' && c <= '9' )
				n = n * (I)radix + (I)( c - '0' );
			else
				break;
			b++;
		}
	}
	numparsed = b - beg;
	if( s )
	{
		if( type_t<I>::is_signed() )
			n = -n;
		else
			numparsed = 0;
	}
	return( n );
}

/// converts string to integer
template <typename I = int>
inline I str2int( char const* buf, uint beg = 0, uint end = type_t<uint>::max(),
                  char radix = 10 )
{
	uint numparsed;
	return( str2int<I>( buf, beg, end, radix, numparsed ) );
}


/// converts string to double, fast implememtation
/** \param numparsed will be filled with length of parsed number (end position = beg + numparsed), this should be checked to determine if number was parsed correctly */
double str2doublel( char const* buf, uint const& beg, uint const& end, uint & numparsed )
{
	double n, zz;
	uint i, k;
	bool f;
	lint x = 0;
	lint y = 0;
	double z = 0;
	uint s = 0;
	uint t = 0;
	lint d = 0;
	uint b = beg;
	f = false;
	numparsed = 0;
	uint e = end > b + 41 ? b + 41 : end;

	if( b < e )
	{
		char cc = buf[b];
		if( cc == '+' )
		{
			b++;
			numparsed++;
		} else {
			if( cc == '-' )
			{
				s = 1;
				numparsed++;
				b++;
			}
		}
	}
	while( b < e )
	{
		char c = buf[b];
		if( d )
		{
			if( f )
			{
				if( c > 47 && c < 58 )
				{
					if( z )
						for( i=0, zz=z; i<9; z *= zz, i++ );
					else
						z = 1.0;
					for( i=0, k=c-48; i<k; z *= 10.0, i++ );
				} else {
					b = e-1;
					numparsed--;
				}
			} else {
				if( c > 47 && c < 58 )
				{
					y = y * 10LL + ( c - 48 );
					d *= 10LL;
				} else {
					if( c == 69 || c == 101 )
					{
						f = true;
						if( b+1 < e )
						{
							char cc = buf[b+1];
							if( cc == '+' )
							{
								b++;
								numparsed++;
							} else {
								if( cc == '-' )
								{
									t = 1;
									b++;
									numparsed++;
								}
							}
						}
					} else {
						b = e-1;
						numparsed--;
					}
				}
			}
		} else {
			if( c > 47 && c < 58 )
			{
				x = x * 10LL + ( c - 48 );
			} else {
				if( c == 46 )
				{
					d = 1;
				} else {
					if( c == 69 || c == 101 )
					{
						d = 1;
						f = true;
						if( b+1 < e )
						{
							char cc = buf[b+1];
							if( cc == '+' )
							{
								b++;
								numparsed++;
							} else {
								if( cc == '-' )
								{
									t = 1;
									b++;
									numparsed++;
								}
							}
						}
					} else {
						b = e-1;
						numparsed--;
					}
				}
			}
		}
		numparsed++;
		b++;
	}
	n = x;
	if( d )
		n += (double)y / d;
	if( s )
		n = -n;
	if( f )
	{
		if( z )
		{
			if( t )
				n /= z;
			else
				n *= z;
		} else {
			n = 0.0;
		}
	}

	return( n );
}

/// converts string to double
/**
currently calls\n
inline double str2doublel( const char *buf, const int &beg, const int &end, int &numparsed )
*/
template <typename T> inline double str2doublel( const T *buf, const int &beg = 0, const int &end = -1 )
{
	int n;
	return( str2doublel( buf, beg, end, n ) );
}

/// converts string to double
/**
currently calls\n
inline double str2doublel( const char *buf, const int &beg, const int &end, int &numparsed )
*/
template <typename T> inline double str2double( const T *buf, const int &beg = 0, const int &end = -1 )
{
	int n;
	return( str2doublel( buf, beg, end, n ) );
}

template <typename buf_t, typename num_t>
inline uint f2strn( buf_t *buf, uint buflen, num_t num, uint length = 3, bool binary_units = false,
					bool pad = false, bool alpha_units = true, uint minlength = 0 )
{
	static const char units[] = {'a','f','p','n','u','m',' ','k','m','g','t','p','e'};
	constexpr int unit_0 = 6;
	constexpr int unit_step = 3;
	uint bufpos = 0;
	int unit = 0;
	auto bm = type_t<num_t>::get_bm( num );
	if( ( bm & type_t<num_t>::bm_exp() ) == type_t<num_t>::bm_exp() )
	{
		if( bufpos < buflen )
		{
			if( bm & type_t<num_t>::bm_sign() )
				buf[bufpos++] = '-';
			else
				if( pad )
					buf[bufpos++] = ' ';
		}
		if( bm & type_t<num_t>::bm_nan() )
		{
			if( bufpos < buflen )
				buf[bufpos++] = 'n';
			if( bufpos < buflen )
				buf[bufpos++] = 'a';
			if( bufpos < buflen )
				buf[bufpos++] = 'n';
		} else {
			if( bufpos < buflen )
				buf[bufpos++] = 'i';
			if( bufpos < buflen )
				buf[bufpos++] = 'n';
			if( bufpos < buflen )
				buf[bufpos++] = 'f';
		}
		if( pad )
			for( uint ilength = 2; ilength < length; ilength++ )
				if( bufpos < buflen )
					buf[bufpos++] = ' ';
	} else {
		if( length < 2 )
			length = 2;
		bool sign = false;
		if( num < 0.0 )
		{
			num = -num;
			bm = type_t<num_t>::get_bm( num );
			sign = true;
		}
		if( bufpos < buflen )
		{
			if( sign )
				buf[bufpos++] = '-';
			else
				if( pad )
					buf[bufpos++] = ' ';
		}
		if( !( bm & ( type_t<num_t>::bm_exp() | type_t<num_t>::bm_mant() ) ) )
		{
			if( bufpos < buflen )
				buf[bufpos++] = '0';
			if( pad )
				for( uint ilength = 0; ilength < length; ilength++ )
					if( bufpos < buflen )
						buf[bufpos++] = ' ';
		} else {
			num_t max = length == 2 ? 100.0 : 1000.0;
			if( binary_units )
			{
				constexpr uint div = 1024;
				if( num > max )
				{
					for( unit=0; num >= max; num /= div, unit++ );
				} else {
					num_t num_mult = num * div;
					for( unit=0; num_mult < max; num = num_mult, num_mult *= div, unit-- );
				}
			} else {
				constexpr uint div = 1000;
				// FIXME: not precise calculation
				if( num > max )
				{
					for( unit=0; num >= max; num /= div, unit++ );
				} else {
					num_t num_mult = num * div;
					for( unit=0; num_mult < max; num = num_mult, num_mult *= div, unit-- );
				}
			}
			
			uint numlength = 0;
			bm = type_t<num_t>::get_bm( num );
			int exp = (int)( ( bm & type_t<num_t>::bm_exp() ) >> type_t<num_t>::numbits_mant() ) - 
			          (int)type_t<num_t>::setbits( type_t<num_t>::numbits_exp() - 1 );
			if( exp < 0 )
			{
				exp = -exp;
				bm = ( bm & type_t<num_t>::bm_mant() ) >> (exp-1);
				numlength += i2strn( buf+bufpos, buflen-bufpos,
				                     (bm>>type_t<num_t>::numbits_mant()) + (1u>>exp) );
			} else {
				bm = ( bm & type_t<num_t>::bm_mant() ) << exp;
				numlength += i2strn( buf+bufpos, buflen-bufpos,
				                     (bm>>type_t<num_t>::numbits_mant()) + (1u<<exp) );
			}
			bufpos += numlength;
			if( numlength < length )
			{
				if( bufpos < buflen )
					buf[bufpos++] = '.';
				constexpr uint radix = 10;
				constexpr uint digits = 3;
				constexpr decltype( bm ) mult = iexp( radix, digits );
				static_assert( type_t<num_t>::bm_mant() * mult <= 
				               ( type_t<num_t>::bm_mant() | type_t<num_t>::bm_exp() | type_t<num_t>::bm_sign() ) );
				while( numlength < length && bufpos + digits <= buflen && bufpos <= buflen - digits )
				{
					static_assert( type_t<num_t>::bm_mant() & 1 );
					static_assert( type_t<num_t>::bm_mant() * mult <= type_t<num_t>::setbits( type_t<num_t>::numbits() - 1 ) );
					bm = ( bm & type_t<num_t>::bm_mant() ) * mult;
					uint num_for_digits = (uint)( bm >> type_t<num_t>::numbits_mant() );
					for( uint k = digits; k; k-- )
					{
						buf[bufpos+k-1] = (char)( num_for_digits % radix ) + '0';
						num_for_digits = num_for_digits / radix;
					}
					numlength += digits;
					bufpos += digits;
				}
				if( numlength > length && numlength - length <= bufpos)
					bufpos -= numlength - length;
			} else {
				if( pad )
					if( bufpos < buflen )
						buf[bufpos++] = '.';
			}
		}
	}
	if( unit )
	{
		int unit_idx = unit + unit_0;
		if( !alpha_units || unit_idx < 0 || unit_idx >= (int)lenof( units ) )
		{
			unit *= unit_step;
			buf[bufpos++] = 'e';
			bufpos += i2strn( buf+bufpos, buflen-bufpos, unit );
		} else {
			buf[bufpos++] = units[unit_idx];
		}
	} else {
		if( pad )
			if( alpha_units )
				buf[bufpos++] = units[unit_0];
	}
	if( bufpos == buflen )
	{
		if( bufpos )
			buf[--bufpos] = '\0';
	} else {
		buf[bufpos] = '\0';
		if( minlength >= buflen )
			minlength = buflen - 1;
		if( minlength > bufpos )
			bufpos = strpad( buf, bufpos, minlength );
	}
	return( bufpos );
}

template <typename buf_t, typename num_t>
uint time2strn( buf_t* buf, uint buflen, num_t time, uint length = 4, uint minlength = 0 )
{
	typedef decltype( type_t<num_t>::bm_mant() ) bm_t;
	uint bufpos = 0;
	bool sign = false;
	num_t seconds = time;
	if( seconds < 0 )
	{
		seconds = -seconds;
		if( buflen )
		{
			*buf = '-';
			buf++;
			buflen--;
		}
		sign = true;
	}
	bm_t int_seconds = (bm_t)seconds % 60;
	num_t minutes = seconds/60;
	bm_t int_minutes = (bm_t)minutes % 60;
	num_t hours = minutes/60;
	bm_t int_hours = (bm_t)hours % 24;
	num_t days = hours/24;
	bm_t int_days = (bm_t)days % 7;
	num_t weeks = days/7;
	bm_t int_weeks = (bm_t)weeks;

	static char str[34];
	if( length < 2 )
		length = 2;
	uint unit = 0;
	if( int_weeks > 0 )
	{
		unit = 5;
		bufpos += i2strn( buf + bufpos, buflen - bufpos, int_weeks );
	}
	if( ( bufpos + 1 < length || ( bufpos == 0 && bufpos < length ) ) && int_days > 0 )
	{
		if( unit == 0 )
			unit = 4;
		if( bufpos > 0 )
			if( bufpos < buflen )
				buf[bufpos++] = ':';
		bufpos += i2strn( buf + bufpos, buflen - bufpos, int_days );
	}
	if( ( bufpos + 2 < length || ( bufpos == 0 && bufpos + 1 < length ) ) && ( int_hours > 0 || unit ) )
	{
		if( unit == 0 )
			unit = 3;
		if( bufpos > 0 )
			if( bufpos < buflen )
				buf[bufpos++] = ':';
		uint strlen = i2strn( str, lenof(str), int_hours );
		if( ( bufpos > 0 || bufpos + 5 < length ) && strlen == 1 )
			if( bufpos < buflen )
				buf[bufpos++] = '0';
		bufpos += str2strn( buf + bufpos, buflen - bufpos, str );
	}
	if( ( bufpos + 2 < length || ( bufpos == 0 && bufpos + 1 < length ) ) && ( int_minutes > 0 || unit ) )
	{
		if( unit == 0 )
			unit = 2;
		if( bufpos > 0 )
			if( bufpos < buflen )
				buf[bufpos++] = ':';
		uint strlen = i2strn( str, lenof(str), int_minutes );
		if( ( bufpos > 0 || bufpos + 5 < length ) && strlen == 1 )
			if( bufpos < buflen )
				buf[bufpos++] = '0';
		bufpos += str2strn( buf + bufpos, buflen - bufpos, str );
	}
	if( bufpos + 2 < length || ( bufpos == 0 && bufpos + 1 < length ) )
	{
		if( unit == 0 )
		{
			unit = 1;
			bufpos += f2strn( buf + bufpos, buflen - bufpos, seconds, length - 1, false, true );
		} else {
			if( bufpos > 0 )
				if( bufpos < buflen )
					buf[bufpos++] = ':';
			uint strlen = i2strn( str, lenof(str), int_seconds );
			if( bufpos > 0 && strlen == 1 )
				if( bufpos < buflen )
					buf[bufpos++] = '0';
			bufpos += str2strn( buf + bufpos, buflen - bufpos, str );
			if( bufpos + 1 < length )
			{
				if( bufpos < buflen )
					buf[bufpos++] = '.';
				num_t frac_seconds = seconds - (bm_t)seconds;
				for( uint i = 0; i < length - bufpos; frac_seconds *= 10.0, i++ );
				bufpos += i2strn( buf + bufpos, buflen - bufpos, (bm_t)frac_seconds,
				                  0, length - bufpos );
			}
		}
	}
	char units[] = {' ','s','m','h','d','w'};
	if( unit > 0 && unit < lenof(units) )
		if( bufpos < buflen )
			buf[bufpos++] = units[unit];
	
	if( bufpos == buflen )
	{
		if( bufpos )
			buf[--bufpos] = '\0';
	} else {
		buf[bufpos] = '\0';
		if( minlength >= buflen )
			minlength = buflen - 1;
		if( minlength > bufpos )
			bufpos = strpad( buf, bufpos, minlength );
	}
	if( sign )
		if( buflen )
			bufpos++;
	return( bufpos );
}

/// copies a string to a string buffer
/** \param buf string buffer \param str string \return number of elements added to string buffer (excluding terminating null character) */
/*
template <typename T> int time2str( T *buf, double time, int length = 4 )
{
	int is, im, ih, id, iw, s;
	double ds, dm, dh, dd, dw, d;
	s = false;
	ds = time;
	if( ds < 0 )
	{
		ds = -ds;
		*(buf++) = '-';
		s = true;
	}
	is = ((int)ds)%60;
	dm = ds/60;
	im = ((int)dm)%60;
	dh = dm/60;
	ih = ((int)dh)%24;
	dd = dh/24;
	id = ((int)dd)%7;
	dw = dd/7;
	iw = (int)dw;

	int l, u, sl, i;
	static char str[34];
	if( length < 2 )
		length = 2;
	u = 0;
	l = 0;
	if( iw > 0 )
	{
		u = 5;
		l += int2str( buf+l, iw );
	}
	if( ( l+1 < length || ( l == 0 && l < length ) ) && id > 0 )
	{
		if( u == 0 )
			u = 4;
		if( l > 0 )
			*(buf+l++) = ':';
		l += int2str( buf+l, id );
	}
	if( ( l+2 < length || ( l == 0 && l+1 < length ) ) && ( ih > 0 || u ) )
	{
		if( u == 0 )
			u = 3;
		if( l > 0 )
			*(buf+l++) = ':';
		sl = int2str( str, ih );
		if( ( l > 0 || l+5 < length ) && sl == 1 )
			*(buf+l++) = '0';
		for( i=0; i<sl; i++ )
			*(buf+l++) = *(str+i);
	}
	if( ( l+2 < length || ( l == 0 && l+1 < length ) ) && ( im > 0 || u ) )
	{
		if( u == 0 )
			u = 2;
		if( l > 0 )
			*(buf+l++) = ':';
		sl = int2str( str, im );
		if( ( l > 0 || l+5 < length ) && sl == 1 )
			*(buf+l++) = '0';
		for( i=0; i<sl; i++ )
			*(buf+l++) = *(str+i);
	}
	if( l+2 < length || ( l == 0 && l+1 < length ) )
	{
		if( u == 0 )
		{
			u = 1;
			l += double2str( buf+l, ds, length-1, 1000 );
		} else {
			if( l > 0 )
				*(buf+l++) = ':';
			sl = int2str( str, is );
			if( l > 0 && sl == 1 )
				*(buf+l++) = '0';
			for( i=0; i<sl; i++ )
				*(buf+l++) = *(str+i);
			if( l+1 < length )
			{
				*(buf+l++) = '.';
				d = ds - (int)ds;
				for( i=0; i<length-l; d*=10.0, i++ );
				l += int2str( buf+l, (int)d, 10, length-l );
			}
		}
	}
	switch( u )
	{
		case 1:
			*(buf+l++) = 's';
			break;
		case 2:
			*(buf+l++) = 'm';
			break;
		case 3:
			*(buf+l++) = 'h';
			break;
		case 4:
			*(buf+l++) = 'd';
			break;
		case 5:
			*(buf+l++) = 'w';
			break;
	}
	*(buf+l) = '\0';
	if( s )
		l++;
	return( l );
}
*/

uint print_time( char * buf, uint buflen, double time );

uint d_fstrn_internal( char* buf, uint buflen,
                       char const* formatedstring, va_list & arguments )
{
	uint bufpos = 0;
	if( buf )
	{
		if( formatedstring )
		{
			constexpr uint maxnumargs = 5;
			int args[maxnumargs];
			bool argsset[maxnumargs];
			uint formatpos = 0;
			while( char c = formatedstring[formatpos++] )
			{
				if( c == '%' )
				{
					uint numargs = 0;
					uint argbeg = formatpos;
					for( ; ( c = formatedstring[formatpos] ); formatpos++ )
					{
						if( ( c < '0' || c > '9' ) && c != '-' && c != '*' )
						{
							if( formatpos > argbeg )
							{
								int iarg;
								if( formatpos - argbeg == 1 && formatedstring[argbeg] == '*' )
									iarg = va_arg( arguments, int );
								else
									iarg = str2int( formatedstring, argbeg, formatpos );
								if( numargs < maxnumargs )
								{
									args[numargs] = iarg;
									argsset[numargs++] = true;
								}
							} else {
								if( numargs < maxnumargs )
									argsset[numargs++] = false;
							}
							formatpos++;
							if( c != ',' )
								break;
							argbeg = formatpos;
						}
					}
#define DEFARGS( lenof_defargs, ... ) \
	int defargs[] = __VA_ARGS__; \
	static_assert( lenof_defargs <= lenof(args) ); \
	for( uint i = 0; i < lenof_defargs; i++ ) \
		if( !argsset[i] || i >= numargs ) \
			args[i] = defargs[i];
					switch( c )
					{
						case '%':
							if( bufpos < buflen )
								buf[bufpos++] = '%';
							break;
						case 'c': // character
							if( bufpos < buflen )
								buf[bufpos++] = va_arg( arguments, uint );
							break;
						case 's': // string
						{
							DEFARGS( 3, { -1, 0, ' ' } );
							char const* str = va_arg( arguments, char* );
							if( !str )
								str = "null";
							int len = args[0] < 0 ? buflen - bufpos : d_min( buflen - bufpos, (uint)args[0] + 1 );
							uint strlen = str2strn( buf + bufpos, len, str );
							if( args[1] < 0 && (uint)(-args[1]) > strlen )
							{
								strpad( buf + bufpos, strlen, (uint)(-args[1]), (char)args[2] );
								strlen = (uint)(-args[1]);
							}
							bufpos += strlen;
							if( args[1] > 0 && (uint)args[1] > strlen )
							{
								uint i = d_min( (uint)args[1] - strlen, buflen - bufpos );
								while( i-- > 0 )
									buf[bufpos++] = (char)args[2];
							}
							break;
						}
						case 'S': // string
						{
							DEFARGS( 3, { -1, 0, ' ' } );
							cstr_t str = va_arg( arguments, cstr_t );
							if( !str.ptr )
								str = "null";
							int len = args[0] < 0
							        ? d_min( str.len, buflen - bufpos )
							        : d_min( d_min( str.len, buflen - bufpos ), (uint)args[0] + 1 );
							uint strlen = str2strn( buf + bufpos, len, str.ptr );
							if( args[1] < 0 && (uint)(-args[1]) > strlen )
							{
								strpad( buf + bufpos, strlen, (uint)(-args[1]), (char)args[2] );
								strlen = (uint)(-args[1]);
							}
							bufpos += strlen;
							if( args[1] > 0 && (uint)args[1] > strlen )
							{
								uint i = d_min( (uint)args[1] - strlen, buflen - bufpos );
								while( i-- > 0 )
									buf[bufpos++] = (char)args[2];
							}
							break;
						}
						case 'x': // hex
						{
							DEFARGS( 1, { 4 } );
							byte const* str = va_arg( arguments, byte* );
							if( !str )
								bufpos += str2strn( buf + bufpos, buflen - bufpos, "null" );
							else
								for( int i = 0; i < args[0]; i++ )
									bufpos += i2strn<byte>( buf + bufpos, buflen - bufpos,
									                        str[i], 2, 2, 16 );
							break;
						}
						case 'i': // integer
						{
							DEFARGS( 4, { 0, 0, 10, -1 } );
							bufpos += i2strn<int>( buf + bufpos, buflen - bufpos,
							                       va_arg( arguments, int ), args[0],
							                       args[1], args[2], args[3] );
							break;
						}
						case 'l': // long long integer
						case 'I': // long long integer
						{
							DEFARGS( 4, { 0, 0, 10, -1 } );
							bufpos += i2strn<lint>( buf + bufpos, buflen - bufpos,
							                        va_arg( arguments, lint ), args[0],
							                        args[1], args[2], args[3] );
							break;
						}
						case 'u': // unsigned integer
						{
							DEFARGS( 4, { 0, 0, 10, -1 } );
							bufpos += i2strn<uint>( buf + bufpos, buflen - bufpos,
							                        va_arg( arguments, uint ), args[0],
							                        args[1], args[2], args[3] );
							break;
						}
						case 'v': // unsigned long long integer
						case 'U': // unsigned long long integer
						{
							DEFARGS( 4, { 0, 0, 10, -1 } );
							bufpos += i2strn<ulint>( buf + bufpos, buflen - bufpos,
							                         va_arg( arguments, ulint ), args[0],
							                         args[1], args[2], args[3] );
							break;
						}
						case 'f': // fixed point integer
						{
							DEFARGS( 2, { 0, 0 } );
							bufpos += i2strn<int>( buf + bufpos, buflen - bufpos,
							                       va_arg( arguments, int ),
							                       args[0] >= 0 ? args[0] + 1 + args[1] : 0,
							                       args[0] >= 0 ? args[0] + 2 : 0,
							                       10,
							                       args[0] );
							break;
						}
						case 'F': // fixed point integer
						{
							DEFARGS( 2, { 0, 0 } );
							bufpos += i2strn<lint>( buf + bufpos, buflen - bufpos,
							                       va_arg( arguments, lint ),
							                       args[0] >= 0 ? args[0] + 1 + args[1] : 0,
							                       args[0] >= 0 ? args[0] + 2 : 0,
							                       10,
							                       args[0] );
							break;
						}
						case 'r': // rectclass (pointer)
						{
							DEFARGS( 3, { 0, 0, 10 } );
							rectclass* r = va_arg( arguments, rectclass* );
							bufpos += i2strn<decltype(r->x)>( buf + bufpos, buflen - bufpos, r->x, args[0], args[1], args[2] );
							bufpos += str2strn( buf + bufpos, buflen - bufpos, "x" );
							bufpos += i2strn<decltype(r->y)>( buf + bufpos, buflen - bufpos, r->y, args[0], args[1], args[2] );
							bufpos += str2strn( buf + bufpos, buflen - bufpos, "_" );
							bufpos += i2strn<decltype(r->width)>( buf + bufpos, buflen - bufpos, r->width, args[0], args[1], args[2] );
							bufpos += str2strn( buf + bufpos, buflen - bufpos, "x" );
							bufpos += i2strn<decltype(r->height)>( buf + bufpos, buflen - bufpos, r->height, args[0], args[1], args[2] );
							break;
						}
						case 'p': // pointclass (pointer)
						{
							DEFARGS( 3, { 0, 0, 10 } );
							pointclass* p = va_arg( arguments, pointclass* );
							bufpos += i2strn<decltype(p->x)>( buf + bufpos, buflen - bufpos, p->x, args[0], args[1], args[2] );
							bufpos += str2strn( buf + bufpos, buflen - bufpos, "x" );
							bufpos += i2strn<decltype(p->y)>( buf + bufpos, buflen - bufpos, p->y, args[0], args[1], args[2] );
							break;
						}
						case 'd': // floating point, alphanummeric units
						{
							DEFARGS( 5, { 3, 0, 0, 1, 0 } );
							bufpos += f2strn( buf + bufpos, buflen - bufpos,
							                  va_arg( arguments, double ),
							                  args[0], args[1] != 0 && args[1] != 1000,
							                  args[2], !args[3]?false:true, args[4] );
							break;
						}
						case 'e': // floating point, exponent
						{
							DEFARGS( 5, { 3, 0, 0, 0, 0 } );
							bufpos += f2strn( buf + bufpos, buflen - bufpos,
							                  va_arg( arguments, double ),
							                  args[0], args[1] != 0 && args[1] != 1000,
							                  args[2], !args[3]?false:true, args[4] );
							break;
						}
						case 'y': // floating point, binary alphanummeric units
						{
							DEFARGS( 5, { 3, 1, 0, 1, 0 } );
							bufpos += f2strn( buf + bufpos, buflen - bufpos,
							                  va_arg( arguments, double ),
							                  args[0], args[1] != 0 && args[1] != 1000,
							                  args[2], !args[3]?false:true, args[4] );
							break;
						}
						case 't': // time
						{
							DEFARGS( 1, { 4 } );
							bufpos += time2strn( buf + bufpos, buflen - bufpos,
							                     va_arg( arguments, double ), args[0] );
							break;
						}
						case 'T': // date
						{
							double d = va_arg( arguments, double );
							if( d )
								bufpos += print_time( buf + bufpos, buflen - bufpos,
													  d );
							else
								bufpos += i2strn<int>( buf + bufpos, buflen - bufpos,
													   0 );
							break;
						}
						case 'b': // boolean
						{
							DEFARGS( 1, { 1 } );
							bool b = (bool)va_arg( arguments, uint );
							char const* str;
							switch( args[0] )
							{
								case 1: str = b ? "true" : "false" ; break;
								case 2: str = b ? "on"   : "off"   ; break;
								case 3: str = b ? "1"    : "0"     ; break;
								case 4: str = b ? "ein"  : "aus"   ; break;
								case 5: str = b ? "okay" : "failed"; break;
								default: str = "";
							}
							bufpos += str2strn( buf + bufpos, buflen - bufpos, str );
							break;
						}
					}
				} else {
					if( bufpos < buflen )
						buf[bufpos++] = c;
				}
			}
		}
		if( bufpos < buflen )
			buf[bufpos] = '\0';
		else
			if( bufpos )
				buf[--bufpos] = '\0';
	}
	return( bufpos );
}

/// converts various valiables to a string using formated string
/** \param buf string to be written \param formatedstring format to be applied to variables \param ... list of variables \return number of characters written \n
	format: %[n1[,n2[..]]]c \n nx: (integer) numbers, c: special character \n
	special characters accessing next variable in argument list (except percentage character): \n
	%: percentage character not using a variable \n
	c: char \n
	s: char* to null terminated string \n
	i: int: using int2str: n1: radix(10), n2: padto(0), n3: minlength(0) \n
	u: uint: using int2str: n1: radix(10), n2: padto(0), n3: splitdist(0) \n
	l: lint: using int2str: n1: radix(10), n2: padto(0), n3: minlength(0) \n
	v: ulint: using int2str: n1: radix(10), n2: padto(0), n3: minlength(0) \n
	r: rectclass*: using int2str: n1: radix(10), n2: padto(0), n3: minlength(0) \n
	p: pointclass*: using int2str: n1: radix(10), n2: padto(0), n3: minlength(0) \n
	d: double: using double2str: n1: length(3), n2: div(1000), n3: pad(0) \n
	f: double: using double2stdstr: n1: digitsbeforedot(2), n2: digitsafterdot(1), n3: padlength(0) \n
	t: double: using time2str: n1: length(4) \n
	b: bool: printing format: n1: 0:true/false, 1:on/off, 2:1/0, 3:an/aus */
/*
template <typename T> int d_fstr( T *buf, const T *formatedstring, ... ) // there are smart and not that smart compilers ..
// [[ deprecated ( "use d_fstrn" ) ]]
int d_fstr( char *buf, const char *formatedstring, ... )
{
	va_list arguments;
	va_start( arguments, formatedstring );
	int j = d_fstrn_internal( buf, type_t<uint>::max(), formatedstring, arguments );
	va_end( arguments );
	return( j );
}
*/

// template <typename T> int d_fstr( T *buf, const T *formatedstring, ... ) // there are smart and not that smart compilers ..
template <typename buf_t> uint d_fstrn( buf_t *buf, uint buflen, buf_t const* formatedstring, ... )
{
	va_list arguments;
	va_start( arguments, formatedstring );
	uint bufpos = d_fstrn_internal( buf, buflen, formatedstring, arguments );
	va_end( arguments );
	return( bufpos );
}

	
#endif // DB_STR_INCLUDED
