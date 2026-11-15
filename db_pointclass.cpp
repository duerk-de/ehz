__GIT_SYMLINKS__
target=../_inc/db_pointclass.cpp
target_resolved=/dibom/src/c/_inc/db_pointclass.cpp
kind=file
sha256=3f804595abdecc86e6584193d1fd61204f5377380102a9aa857ac1240ca8839c
size=7316
created=2026-11-15T10:29:04.652856
--END-HEADER--
#ifndef DB_POINTCLASS_INCLUDED
#define DB_POINTCLASS_INCLUDED


template <typename T> class pointclass_t
{
	public:
		T x;
		T y;

		inline pointclass_t()
		{
			x = 0;
			y = 0;
		}
		template <typename R> inline pointclass_t( pointclass_t<R> const& p )
		{
			x = p.x;
			y = p.y;
		}
		inline pointclass_t( const T & _x, const T & _y )
		{
			x = _x;
			y = _y;
		}
		inline pointclass_t( const T & i )
		{
			x = i;
			y = i;
		}

		inline void set( const T & _x = 0, const T & _y = 0 )
		{
			x = _x;
			y = _y;
		}

		inline T area() const
		{
			return( x*y );
		}

		inline T hi() const
		{
			if( x > y )
				return( x );
			else
				return( y );
		}
		inline T lo() const
		{
			if( x < y )
				return( x );
			else
				return( y );
		}

        inline double length()
        {
        	return( sqrt15( x, y ) );
        }

        inline T sqlength()
        {
        	return( x*x + y*y );
        }

		inline pointclass_t operator - () const
		{
			return( pointclass_t( -x, -y ) );
		}

		inline pointclass_t operator + ( const pointclass_t & p ) const
		{
			return( pointclass_t( x+p.x, y+p.y ) );
		}
		inline pointclass_t & operator += ( const pointclass_t & p )
		{
			x+=p.x; y+=p.y;
			return( *this );
		}

		inline pointclass_t operator - ( const pointclass_t & p ) const
		{
			return( pointclass_t( x-p.x, y-p.y ) );
		}
		inline pointclass_t & operator -= ( const pointclass_t & p )
		{
			x-=p.x; y-=p.y;
			return( *this );
		}

		inline pointclass_t operator * ( const pointclass_t & p ) const
		{
			return( pointclass_t( x*p.x, y*p.y ) );
		}
		inline pointclass_t & operator *= ( const pointclass_t & p )
		{
			x*=p.x; y*=p.y;
			return( *this );
		}

		inline pointclass_t operator / ( const pointclass_t & p ) const
		{
			return( pointclass_t( x/p.x, y/p.y ) );
		}
		inline pointclass_t & operator /= ( const pointclass_t & p )
		{
			x/=p.x; y/=p.y;
			return( *this );
		}

		inline pointclass_t operator % ( const pointclass_t & p ) const
		{
			return( pointclass_t( x%p.x, y%p.y ) );
		}
		inline pointclass_t & operator %= ( const pointclass_t & p )
		{
			x%=p.x; y%=p.y;
			return( *this );
		}

		inline pointclass_t operator + ( const T & i ) const
		{
			return( pointclass_t( x+i, y+i ) );
		}
		inline pointclass_t & operator += ( const T & i )
		{
			x+=i; y+=i;
			return( *this );
		}

		inline pointclass_t operator - ( const T & i ) const
		{
			return( pointclass_t( x-i, y-i ) );
		}
		inline pointclass_t & operator -= ( const T & i )
		{
			x-=i; y-=i;
			return( *this );
		}

		inline pointclass_t operator * ( const T & i ) const
		{
			return( pointclass_t( x*i, y*i ) );
		}
		inline pointclass_t & operator *= ( const T & i )
		{
			x*=i; y*=i;
			return( *this );
		}

		inline pointclass_t operator / ( const T & i ) const
		{
			return( pointclass_t( x/i, y/i ) );
		}
		inline pointclass_t & operator /= ( const T & i )
		{
			x /= i; y /= i;
			return( *this );
		}

		inline pointclass_t operator % ( const T & i ) const
		{
			return( pointclass_t( x % i, y % i ) );
		}
		inline pointclass_t & operator %= ( const T & i )
		{
			x %= i; y %= i;
			return( *this );
		}

		inline pointclass_t operator << ( const T & i ) const
		{
			return( pointclass_t( x << i, y << i ) );
		}
		inline pointclass_t operator >> ( const T & i ) const
		{
			return( pointclass_t( x >> i, y >> i ) );
		}
		inline pointclass_t & operator <<= ( const T & i )
		{
			x <<= i; y <<= i;
			return( *this );
		}
		inline pointclass_t & operator >>= ( const T & i )
		{
			x >>= i; y >>= i;
			return( *this );
		}

		inline pointclass_t & operator = ( const T & i )
		{
			x = i; y = i;
			return( *this );
		}

		inline bool operator < ( const pointclass_t & p ) const
		{
			return( x < p.x && y < p.y );
		}
		inline bool operator < ( const T & i ) const
		{
			return( x < i && y < i );
		}

		inline bool operator > ( const pointclass_t & p ) const
		{
			return( x > p.x && y > p.y );
		}
		inline bool operator > ( const T & i ) const
		{
			return( x > i && y > i );
		}

		inline bool operator <= ( const pointclass_t & p ) const
		{
			return( x <= p.x && y <= p.y );
		}

		inline bool operator >= ( const pointclass_t & p ) const
		{
			return( x >= p.x && y >= p.y );
		}

		inline bool operator == ( const pointclass_t & p ) const
		{
			return( x == p.x && y == p.y );
		}
		inline bool operator == ( const T & i ) const
		{
			return( x == i && y == i );
		}

		inline bool operator != ( const pointclass_t & p ) const
		{
			return( x != p.x || y != p.y );
		}
		inline bool operator != ( const T & i ) const
		{
			return( x != i || y != i );
		}

		inline operator bool() const
		{
			return( x && y );
		}

		static inline double sqrt15( double arg )
		{
			double a, a2;
			if( arg < 0.0 )
			{
				return( 0.0 );
			} else {
				if( arg > 4.0 )
				{
					return( 1.0 );
				} else {
					a = arg - 1.5;
					a2 = a*a;
					return( 1.2247448719315894 + 0.4082482904638629*a - 0.06804138174397717*a2 + 0.02268046058132572*a2*a - 0.009450191908885717*a2*a2 );
				}
			}
		}

		static inline double sqrt15( double swidth, double sheight )
		{
			double swidth2, sheight2, length;
			swidth2 = swidth * swidth;
			sheight2 = sheight * sheight;
            if( !swidth )
            	length = sheight;
            else
            	if( !sheight )
            		length = swidth;
            	else
					if( swidth2 > sheight2 )
						length = sqrt15( ( swidth2 + sheight2 ) / swidth2 ) * ( swidth < 0 ? -swidth : swidth );
					else
						length = sqrt15( ( swidth2 + sheight2 ) / sheight2 ) * ( sheight < 0 ? -sheight : sheight );
			return( length );
		}

};

/*
class doublepointclass
{
	public:
		double x, y;

		inline doublepointclass( const double & d = 0.0 )
		{
			set( d, d );
		}
		inline doublepointclass( const double & _x, const double & _y )
		{
			set( _x, _y );
		}
		inline doublepointclass( const pointclass & p )
		{
			set( (double)p.x, (double)p.y );
		}

		inline void set( const double & _x, const double & _y )
		{
			x = _x;
			y = _y;
		}

		inline pointclass operator() ()
		{
			return( pointclass( (int)x, (int)y ) );
		}

		inline doublepointclass & operator = ( const pointclass & p )
		{
			x = (double)p.x;
			y = (double)p.y;
			return( *this );
		}

		inline doublepointclass operator + ( const double & d ) const
		{
			return( doublepointclass( x + d, y + d ) );
		}
		inline doublepointclass operator - ( const double & d ) const
		{
			return( doublepointclass( x - d, y - d ) );
		}
		inline doublepointclass operator * ( const double & d ) const
		{
			return( doublepointclass( x * d, y * d ) );
		}
		inline doublepointclass operator / ( const double & d ) const
		{
			return( doublepointclass( x / d, y / d ) );
		}
};
*/

typedef pointclass_t<int> pointclass;
typedef pointclass_t<double> doublepointclass;

typedef pointclass_t<int> pt_t;
typedef pointclass_t<double> dbl_pt_t;
typedef pointclass_t<float> flt_pt_t;

#endif // DB_POINTCLASS_INCLUDED
