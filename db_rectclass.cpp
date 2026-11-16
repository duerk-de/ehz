__GIT_SYMLINKS__
target=../_inc/db_rectclass.cpp
target_resolved=/dibom/src/c/_inc/db_rectclass.cpp
kind=file
sha256=a51f893c125794efd3ca026df704c1fe0dda87cabdc0dc4e11f5db1b99c220bc
size=9196
created=2026-11-16T20:49:12.752004
--END-HEADER--
#ifndef DB_RECTCLASS_INCLUDED
#define DB_RECTCLASS_INCLUDED

#include "db_pointclass.cpp"

class rectclass
{
	public:
		int x;
		int y;
		int width;
		int height;

		inline rectclass()
		{
			x = 0;
			y = 0;
			width = 0;
			height = 0;
		}

		inline rectclass( int const& _x, int const& _y, int const& _width, int const& _height )
		{
			x = _x;
			y = _y;
			width = _width;
			height = _height;
		}

		inline rectclass( pointclass const& pos, pointclass const& size )
		{
			x = pos.x;
			y = pos.y;
			width = size.x;
			height = size.y;
		}

		inline rectclass( rectclass const& r )
		{
			x = r.x;
			y = r.y;
			width = r.width;
			height = r.height;
		}

		inline rectclass& operator =( rectclass const& r )
		{
			x = r.x;
			y = r.y;
			width = r.width;
			height = r.height;
			return( *this );
		}

		inline rectclass &set( rectclass const& r )
		{
			x = r.x;
			y = r.y;
			width = r.width;
			height = r.height;
			return( *this );
		}

		inline rectclass &set( pointclass const& pos, pointclass const& size )
		{
			x = pos.x;
			y = pos.y;
			width = size.x;
			height = size.y;
			return( *this );
		}

		inline rectclass &set( int const& _x, int const& _y, int const& _width, int const& _height )
		{
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			return( *this );
		}

		inline rectclass &setorigin( const int & _i )
		{
			x = _i;
			y = _i;
			return( *this );
		}

		inline rectclass &setorigin( const int & _x, const int & _y )
		{
			x = _x;
			y = _y;
			return( *this );
		}

		inline rectclass &setorigin( const pointclass & p )
		{
			x = p.x;
			y = p.y;
            return( *this );
		}

		inline rectclass &setsize( const int & _i )
		{
			width = _i;
			height = _i;
            return( *this );
		}

		inline rectclass &setsize( const int & _width, const int & _height )
		{
			width = _width;
			height = _height;
            return( *this );
		}

		inline rectclass &setsize( const pointclass & p )
		{
			width = p.x;
			height = p.y;
            return( *this );
		}

		inline rectclass shift( const pointclass & p )
		{
            return( rectclass( x + p.x, y + p.y, width, height ) );
		}

		inline rectclass shift( const int & i = 0 )
		{
            return( rectclass( x + i, y + i, width, height ) );
		}

		inline rectclass &setshift( const pointclass & p )
		{
			x += p.x;
			y += p.y;
            return( *this );
		}

		inline rectclass &setshift( const int & i = 0 )
		{
			x += i;
			y += i;
            return( *this );
		}

		inline rectclass resize( const pointclass & p )
		{
            return( rectclass( x, y, width + p.x, height + p.y ) );
		}

		inline rectclass resize( const int & i = 0 )
		{
			return( rectclass( x, y, width + i, height + i ) );
		}

		inline rectclass &setresize( const pointclass & p )
		{
			width += p.x;
			height += p.y;
            return( *this );
		}

		inline rectclass &setresize( const int & i = 0 )
		{
			width += i;
			height += i;
            return( *this );
		}

		inline rectclass &setpoints( const int & _x, const int & _y, const int & _xe, const int & _ye )
		{
			x = _x;
			y = _y;
			width = _xe - _x;
			height = _ye - _y;
            return( *this );
		}

		inline rectclass &setpoints( const pointclass & p1, const pointclass & p2 )
		{
			setpoints( p1.x, p1.y, p2.x, p2.y );
            return( *this );
		}

		inline rectclass &setlowerright( const int & _xe, const int & _ye )
		{
			width = _xe - x;
			height = _ye - y;
            return( *this );
		}

		inline rectclass &setlowerright( const pointclass & p2 )
		{
			setlowerright( p2.x, p2.y );
            return( *this );
		}

		inline pointclass origin() const
		{
			return( pointclass( x, y ) );
		}

		inline pointclass size() const
		{
			return( pointclass( width, height ) );
		}

		inline pointclass lowerright() const
		{
			return( pointclass( x+width, y+height ) );
		}

        inline pointclass center() const
        {
        	return( pointclass( x+width/2, y+height/2 ) );
        }

		inline int area() const
		{
			return( width*height );
		}

		inline rectclass operator + ( const pointclass & p ) const
		{
			return( rectclass( x+p.x, y+p.y, width, height ) );
		}

		inline rectclass operator - ( const pointclass & p ) const
		{
			return( rectclass( x-p.x, y-p.y, width, height ) );
		}

		inline rectclass operator * ( const pointclass & p ) const
		{
			return( rectclass( x*p.x, y*p.y, width*p.x, height*p.y ) );
		}

		inline rectclass operator / ( const pointclass & p ) const
		{
			return( rectclass( x/p.x, y/p.y, width/p.x, height/p.y ) );
		}

		inline rectclass operator >> ( const pointclass & p ) const
		{
			return( rectclass( x>>p.x, y>>p.y, width>>p.x, height>>p.y ) );
		}
		inline rectclass operator << ( const pointclass & p ) const
		{
			return( rectclass( x<<p.x, y<<p.y, width<<p.x, height<<p.y ) );
		}

		inline rectclass operator >> ( const int & i ) const
		{
			return( rectclass( x>>i, y>>i, width>>i, height>>i ) );
		}
		inline rectclass operator << ( const int & i ) const
		{
			return( rectclass( x<<i, y<<i, width<<i, height<<i ) );
		}

		inline rectclass operator + ( const int & i ) const
		{
			return( rectclass( x+i, y+i, width, height ) );
		}

		inline rectclass operator - ( const int & i ) const
		{
			return( rectclass( x-i, y-i, width, height ) );
		}

		inline rectclass operator * ( const int & i ) const
		{
			return( rectclass( x*i, y*i, width*i, height*i ) );
		}

		inline rectclass operator / ( const int & i ) const
		{
			return( rectclass( x/i, y/i, width/i, height/i ) );
		}

		inline rectclass operator * ( const double & d ) const
		{
			return( rectclass( (int)(x*d+0.5), (int)(y*d+0.5), (int)(width*d+0.5), (int)(height*d+0.5) ) );
		}

		inline rectclass operator / ( const double & d ) const
		{
			return( rectclass( (int)(x/d+0.5), (int)(y/d+0.5), (int)(width/d+0.5), (int)(height/d+0.5) ) );
		}

		inline bool operator ! () const
		{
			return( !x && !y && !width && !height );
		}

		inline bool operator == ( const rectclass & r ) const
		{
            return( r.x==x && r.width==width && r.y==y && r.height==height );
		}
		inline bool operator != ( const rectclass & r ) const
		{
            return( r.x!=x || r.width!=width || r.y!=y || r.height!=height );
		}

		inline bool operator == ( const pointclass & p ) const
		{
            return( p.x>=x && p.x<x+width && p.y>=y && p.y<y+height );
		}
		inline bool operator != ( const pointclass & p ) const
		{
            return( p.x<x || p.x>=x+width || p.y<y || p.y>=y+height );
		}

        inline rectclass toarea( pointclass limits )
        {
        	return( toarea( rectclass( pointclass(), limits ) ) );
        }
        inline rectclass &settoarea( pointclass limits )
        {
        	return( settoarea( rectclass( pointclass(), limits ) ) );
        }
        inline rectclass &settoarea( rectclass limits )
        {
        	int right, bottom;
        	right = limits.x + limits.width;
            bottom = limits.y + limits.height;
        	if( x > right )
            	x = right;
        	if( x < limits.x )
            	x = limits.x;
        	if( y > bottom )
            	y = bottom;
        	if( y < limits.y )
            	y = limits.y;
            if( x + width > right )
            	width = right - x;
            if( y + height > bottom )
            	height = bottom - y;
            return( *this );
        }
        inline rectclass toarea( rectclass limits ) const
        {
			rectclass r;

			r = *this;
            int right, bottom;
        	right = limits.x + limits.width;
            bottom = limits.y + limits.height;
        	if( r.x > right )
            	r.x = right;
        	if( r.x < limits.x )
            	r.x = limits.x;
        	if( r.y > bottom )
            	r.y = bottom;
        	if( r.y < limits.y )
            	r.y = limits.y;
            if( r.x + r.width > right )
            	r.width = right - r.x;
            if( r.y + r.height > bottom )
            	r.height = bottom - r.y;
            return( r );
        }

};

class doublerectclass
{
	public:
		double x, y, width, height;

		doublerectclass( const double & d = 0.0 )
		{
			set( d, d, d, d );
		}
		doublerectclass( const pointclass & _origin, const pointclass & _size )
		{
			set( _origin.x, _origin.y, _size.x, _size.y );
		}
		doublerectclass( const doublepointclass & _origin, const doublepointclass & _size )
		{
			set( _origin.x, _origin.y, _size.x, _size.y );
		}
		doublerectclass( const double & _x, const double & _y, const double & _width, const double & _height )
		{
			set( _x, _y, _width, _height );
		}

		inline void set( const double & _x, const double & _y, const double & _width, const double & _height )
		{
			x = _x;
			y = _y;
			width = _width;
			height = _height;
		}
};


#endif // DB_RECTCLASS_INCLUDED
