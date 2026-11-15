__GIT_SYMLINKS__
target=../_inc/db_time.cpp
target_resolved=/dibom/src/c/_inc/db_time.cpp
kind=file
sha256=77b7bd1781bc772bd9b7cde8033446f61fe58184a4332926e0fdf323cf49a330
size=31021
created=2026-11-15T10:29:04.660396
--END-HEADER--
#ifndef DB_TIME_INCLUDED
#define DB_TIME_INCLUDED

#include "db_defines.cpp"
#include "db_mem.cpp"
#include "db_str.cpp"


/*
template <typename t>
struct nsecs_from
{
	static constexpr ulint nsecs( t v = 1 ) { return(                           (ulint)v          ); }
	static constexpr ulint usecs( t v = 1 ) { return( nsecs_from<ulint>::nsecs( (ulint)v * 1000 ) ); }
	static constexpr ulint msecs( t v = 1 ) { return( nsecs_from<ulint>::usecs( (ulint)v * 1000 ) ); }
	static constexpr ulint secs ( t v = 1 ) { return( nsecs_from<ulint>::msecs( (ulint)v * 1000 ) ); }
	static constexpr ulint mins ( t v = 1 ) { return( nsecs_from<ulint>::secs ( (ulint)v *   60 ) ); }
	static constexpr ulint hours( t v = 1 ) { return( nsecs_from<ulint>::mins ( (ulint)v *   60 ) ); }
	static constexpr ulint days ( t v = 1 ) { return( nsecs_from<ulint>::hours( (ulint)v *   24 ) ); }
	static constexpr ulint weeks( t v = 1 ) { return( nsecs_from<ulint>::days ( (ulint)v *    7 ) ); }
};

template <typename t>
struct nsecs_to
{
	static constexpr t nsecs( ulint v = 1 ) { return(                         v          ); }
	static constexpr t usecs( ulint v = 1 ) { return( nsecs_to<ulint>::nsecs( v / 1000 ) ); }
	static constexpr t msecs( ulint v = 1 ) { return( nsecs_to<ulint>::usecs( v / 1000 ) ); }
	static constexpr t secs ( ulint v = 1 ) { return( nsecs_to<ulint>::msecs( v / 1000 ) ); }
	static constexpr t mins ( ulint v = 1 ) { return( nsecs_to<ulint>::secs ( v /   60 ) ); }
	static constexpr t hours( ulint v = 1 ) { return( nsecs_to<ulint>::mins ( v /   60 ) ); }
	static constexpr t days ( ulint v = 1 ) { return( nsecs_to<ulint>::hours( v /   24 ) ); }
	static constexpr t weeks( ulint v = 1 ) { return( nsecs_to<ulint>::days ( v /    7 ) ); }
};
*/
template <typename t>
struct nsecs_mult
{
	static constexpr t nsec() { return(                          (t)1    ); }
	static constexpr t usec() { return( nsecs_mult<t>::nsec() * (t)1000 ); }
	static constexpr t msec() { return( nsecs_mult<t>::usec() * (t)1000 ); }
	static constexpr t sec () { return( nsecs_mult<t>::msec() * (t)1000 ); }
	static constexpr t min () { return( nsecs_mult<t>::sec () * (t)  60 ); }
	static constexpr t hour() { return( nsecs_mult<t>::min () * (t)  60 ); }
	static constexpr t day () { return( nsecs_mult<t>::hour() * (t)  24 ); }
	static constexpr t week() { return( nsecs_mult<t>::day () * (t)   7 ); }
};


// capacity: uint: 100 years, short: hour, byte: min
template <typename t>
struct sec_t
{
	static constexpr t sec  = nsecs_mult<ulint>::sec () / nsecs_mult<ulint>::sec ();
	static constexpr t min  = nsecs_mult<ulint>::min () / nsecs_mult<ulint>::sec ();
	static constexpr t hour = nsecs_mult<ulint>::hour() / nsecs_mult<ulint>::sec ();
	static constexpr t day  = nsecs_mult<ulint>::day () / nsecs_mult<ulint>::sec ();
	static constexpr t week = nsecs_mult<ulint>::week() / nsecs_mult<ulint>::sec ();
};

// capacity: uint: week, short min, ulint: 5e8 years
template <typename t>
struct msec_t
{
	static constexpr t msec = nsecs_mult<ulint>::msec() / nsecs_mult<ulint>::msec ();
	static constexpr t sec  = nsecs_mult<ulint>::sec () / nsecs_mult<ulint>::msec ();
	static constexpr t min  = nsecs_mult<ulint>::min () / nsecs_mult<ulint>::msec ();
	static constexpr t hour = nsecs_mult<ulint>::hour() / nsecs_mult<ulint>::msec ();
	static constexpr t day  = nsecs_mult<ulint>::day () / nsecs_mult<ulint>::msec ();
	static constexpr t week = nsecs_mult<ulint>::week() / nsecs_mult<ulint>::msec ();
};

// capacity: uint: hour, short msec, ulint: 1e5 years
template <typename t>
struct usec_t
{
	static constexpr t usec = nsecs_mult<ulint>::usec() / nsecs_mult<ulint>::usec ();
	static constexpr t msec = nsecs_mult<ulint>::msec() / nsecs_mult<ulint>::usec ();
	static constexpr t sec  = nsecs_mult<ulint>::sec () / nsecs_mult<ulint>::usec ();
	static constexpr t min  = nsecs_mult<ulint>::min () / nsecs_mult<ulint>::usec ();
	static constexpr t hour = nsecs_mult<ulint>::hour() / nsecs_mult<ulint>::usec ();
	static constexpr t day  = nsecs_mult<ulint>::day () / nsecs_mult<ulint>::usec ();
	static constexpr t week = nsecs_mult<ulint>::week() / nsecs_mult<ulint>::usec ();
};

template <typename t>
struct nsec_t
{
	static constexpr t nsec = nsecs_mult<ulint>::nsec() / nsecs_mult<ulint>::nsec ();
	static constexpr t usec = nsecs_mult<ulint>::usec() / nsecs_mult<ulint>::nsec ();
	static constexpr t msec = nsecs_mult<ulint>::msec() / nsecs_mult<ulint>::nsec ();
	static constexpr t sec  = nsecs_mult<ulint>::sec () / nsecs_mult<ulint>::nsec ();
	static constexpr t min  = nsecs_mult<ulint>::min () / nsecs_mult<ulint>::nsec ();
	static constexpr t hour = nsecs_mult<ulint>::hour() / nsecs_mult<ulint>::nsec ();
	static constexpr t day  = nsecs_mult<ulint>::day () / nsecs_mult<ulint>::nsec ();
	static constexpr t week = nsecs_mult<ulint>::week() / nsecs_mult<ulint>::nsec ();
};


#ifdef LINUX

	#include <time.h>
	#include <sys/time.h>
	#include <unistd.h>

    inline ulint getintnanos()
    {
        struct timespec ts;
        clock_gettime( CLOCK_MONOTONIC, &ts );
        return( (ulint)ts.tv_sec * 1000000000ull + (ulint)ts.tv_nsec );
    }
	/// returns operating system specific (internal) actual time in seconds
	inline double getintseconds()
	{
		struct timespec ts;
		clock_gettime( CLOCK_MONOTONIC, &ts );
		return( (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9 );
	}
	/// returns time in seconds since the epoch
	inline ulint getnanos()
	{
		struct timespec ts;
		clock_gettime( CLOCK_REALTIME, &ts );
        return( (ulint)ts.tv_sec * 1000000000ull + (ulint)ts.tv_nsec );
	}
	/// returns time in seconds since the epoch
	inline double getseconds()
	{
		struct timespec ts;
		clock_gettime( CLOCK_REALTIME, &ts );
		return( (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9 );
	}
	/// sleeps for at least specified time
	static inline void sleepus( int const& microseconds )
	{
		if( microseconds > 0 )
			usleep( microseconds );
	}
	static inline void sleepus( uint const& microseconds )
	{
		usleep( microseconds );
	}
	/// sleeps for at least specified time
	static inline void sleepms( int milliseconds )
	{
		if( milliseconds > 0 )
			usleep( milliseconds*1000 );
	}
	/// sleeps for at least specified time
	static inline void sleeps( double seconds )
	{
		int us;
		if( seconds > 0.0 )
		{
			us = (int)(seconds*1e6);
			if( us > 1 )
				usleep( us );
			else
				usleep( 1 );
		}
	}

#else
	
	#include <windows.h>
	
	/// returns operating system specific (internal) actual time of the epoch
	ulint getfiletimeoffset()
	{
	    SYSTEMTIME systemtime;
	    FILETIME filetime;
	
	    systemtime.wYear = 1970;
	    systemtime.wMonth = 1;
	    systemtime.wDay = 1;
	    systemtime.wHour = 0;
	    systemtime.wMinute = 0;
	    systemtime.wSecond = 0;
	    systemtime.wMilliseconds = 0;
	    SystemTimeToFileTime( &systemtime, &filetime );
	    
	    return( ( ( (ulint)filetime.dwHighDateTime ) << 32 ) | (ulint)filetime.dwLowDateTime );
	}
	/// returns operating system specific (internal) actual time in seconds
	inline double getintseconds()
	{
		LARGE_INTEGER li_frequency, li_time;
		static lint frequency = 0ll;
		
		if( !frequency )
		{
			QueryPerformanceFrequency( &li_frequency );
			frequency = li_frequency.QuadPart;
		}
		QueryPerformanceCounter( &li_time );
		if( frequency )
			return( (double)li_time.QuadPart / (double)frequency );
		else
			return( 0.0 );
	}
	/// returns time in seconds since the epoch
	inline double getseconds()
	{
		static ulint filetimeoffset = -1ull;
		FILETIME filetime;
		
		if( filetimeoffset == -1ull )
			filetimeoffset = getfiletimeoffset();
		GetSystemTimeAsFileTime( &filetime );
		
		return( (double)( ( ( ( (ulint)filetime.dwHighDateTime ) << 32 ) | (ulint)filetime.dwLowDateTime ) - filetimeoffset ) * 1e-7 );
	}
	/// sleeps for at least specified time
	static inline void sleepms( int milliseconds )
	{
		if( milliseconds > 0 )
			Sleep( milliseconds );
	}
	/// sleeps for at least specified time
	static inline void sleepus( int microseconds )
	{
		int ms;
		if( microseconds > 0 )
		{
			ms = microseconds/1000;
			if( ms > 0 )
				Sleep( ms );
			else
				Sleep( 1 );
		}
	}
	/// sleeps for at least specified time
	static inline void sleeps( double seconds )
	{
		int ms;
		if( seconds > 0.0 )
		{
			ms = (int)(seconds*1e3);
			if( ms > 1 )
				Sleep( ms );
			else
				Sleep( 1 );
		}
	}


#endif


////////////////////////////////////////////////////////////////////////////////

/// allocates and returns buffer holding a copy of string
/** \param string string to copy \param length [out] length of string and buffer \return pointer to buffer */
template <typename T> inline T *makebuf( const T *string, int &length )
{
	T *buf;
	buf = null;
	if( string )
	{
		length = d_strlen( string );
		if( ( buf = new (std::nothrow) T[length+1] ) )
			cpy( buf, string, (length+1)*sizeof(T) );
		else
			length = 0;
	}
	return( buf );
}
/// allocates and returns buffer holding a copy of string
/** \param string string to copy \return pointer to buffer */
template <typename T> inline T *makebuf( const T *string )
{
	int l;
	return( makebuf( string, l ) );
}
/// compares up to maxlength bytes of two strings
/** \param string null terminated string 1 \param string null terminated string 2 \param maxlength maximum length to compare \return true if strings are equal (at least up to maxlength characters) */
template <typename T> bool d_strcmp( const T *str1, const T *str2, int maxlength = -1 )
{
	int i = -1;
	T const *c1 = str1 - 1;
	T const *c2 = str2 - 1;
	if( maxlength < 0 )
		while( *++c1 && *++c2 && *c1 == *c2 );
	else
		while( ++i<maxlength && *++c1 && *++c2 && *c1 == *c2 );
	if( !*c1 )
		++c2;
	return( ( !*c1 && !*c2 ) || ( !(maxlength<0) && i==maxlength ) );
}

/// class provides formatting of a timestamp
class timefclass
{
public:
	static const int TIMEFCLASS_DEF_TIMEZONE = 1;
	int year;			///< year, should be >= 1970
	int month;			///< month of year (1...12)
	int day;			///< day (1...31)
	int hour;			///< hour (0...23)
	int min;			///< minute (0...59)
	int sec;			///< second (0...59)
	int nsec;			///< nanosecond (0...999999999)
	int dayofweek;		///< day of week 0...6 (mon...fri)
	int week;			///< week of year
	double time;		///< time since epoch in seconds
	int timezone;		///< timezone formated time calculated for
	int summertime;		///< is summertime
	
	astream <char> sdow, smon;
	char *sempty;
	
	void init()
	{
		year = month = day = hour = min = sec = nsec = dayofweek = week = 0;
		time = 0.0;
		summertime = 0;
		timezone = TIMEFCLASS_DEF_TIMEZONE;
		
		sempty = makebuf( "" );
		
		sdow.setcl( 7 );
		sdow.add( makebuf( "monday" ) );
		sdow.add( makebuf( "tuesday" ) );
		sdow.add( makebuf( "wednesday" ) );
		sdow.add( makebuf( "thursday" ) );
		sdow.add( makebuf( "friday" ) );
		sdow.add( makebuf( "saturday" ) );
		sdow.add( makebuf( "sunday" ) );
		sdow.add( makebuf( "mon" ) );
		sdow.add( makebuf( "tue" ) );
		sdow.add( makebuf( "wed" ) );
		sdow.add( makebuf( "thu" ) );
		sdow.add( makebuf( "fri" ) );
		sdow.add( makebuf( "sat" ) );
		sdow.add( makebuf( "sun" ) );
		sdow.add( makebuf( "mo" ) );
		sdow.add( makebuf( "tu" ) );
		sdow.add( makebuf( "we" ) );
		sdow.add( makebuf( "th" ) );
		sdow.add( makebuf( "fr" ) );
		sdow.add( makebuf( "sa" ) );
		sdow.add( makebuf( "su" ) );
		
		smon.setcl( 12 );
		smon.add( makebuf( "january" ) );
		smon.add( makebuf( "feburary" ) );
		smon.add( makebuf( "march" ) );
		smon.add( makebuf( "april" ) );
		smon.add( makebuf( "may" ) );
		smon.add( makebuf( "june" ) );
		smon.add( makebuf( "july" ) );
		smon.add( makebuf( "august" ) );
		smon.add( makebuf( "september" ) );
		smon.add( makebuf( "october" ) );
		smon.add( makebuf( "november" ) );
		smon.add( makebuf( "december" ) );
		smon.add( makebuf( "jan" ) );
		smon.add( makebuf( "feb" ) );
		smon.add( makebuf( "mar" ) );
		smon.add( makebuf( "apr" ) );
		smon.add( makebuf( "may" ) );
		smon.add( makebuf( "jun" ) );
		smon.add( makebuf( "jul" ) );
		smon.add( makebuf( "aug" ) );
		smon.add( makebuf( "sep" ) );
		smon.add( makebuf( "oct" ) );
		smon.add( makebuf( "nov" ) );
		smon.add( makebuf( "dec" ) );
		smon.add( makebuf( "ja" ) );
		smon.add( makebuf( "fe" ) );
		smon.add( makebuf( "ma" ) );
		smon.add( makebuf( "ap" ) );
		smon.add( makebuf( "ma" ) );
		smon.add( makebuf( "ju" ) );
		smon.add( makebuf( "jl" ) );
		smon.add( makebuf( "au" ) );
		smon.add( makebuf( "se" ) );
		smon.add( makebuf( "oc" ) );
		smon.add( makebuf( "no" ) );
		smon.add( makebuf( "de" ) );
	}
	
public:
	/// constructor
	timefclass()
	{
		init();
	}
	timefclass( double const& _time )
	{
		init();
		set( _time );
	}
	/// destructor
	~timefclass()
	{
		dropa( sempty );
	}
	/// returns pointer to a string, describing day of week (dow)
	char *getsdow( int _dow, int _type ) const
	{
		int i;
		i = _dow + _type * 7;
		if( i > -1 && (uint)i < sdow.num() )
			return( sdow.get( i ) );
		else
			return( sempty );
	}
	/// returns pointer to a string, describing month (mon)
	char *getsmon( int _mon, int _type ) const
	{
		int i;
		i = _mon - 1 + _type * 12;
		if( i > -1 && (uint)i < smon.num() )
			return( smon.get( i ) );
		else
			return( sempty );
	}
	/// explicitely sets formatted time
	void set( int _year, int _month, int _day, int _hour, int _min, int _sec, int _nsec, int _timezone, int _summertime, bool autosummertime )
	{
		lint totaldays, daysbeginningofyear;
		int daysperyear;
		
		timezone = _timezone;
		summertime = _summertime;
		
		totaldays = ( timezone + summertime ) / 24;
		
		year = 1970;
		while( year < _year )
		{
			daysperyear = 365;
			if( !( year % 4 ) && ( ( year % 100 ) || !( year % 400 ) ) )
				daysperyear++;
			totaldays += daysperyear;
			year++;
		}

		daysbeginningofyear = totaldays;

		month = _month;
		if( month > 1 )
		{
			if( month < 3 )
				totaldays += 31;
			else if( month < 4 )
				totaldays += 59;
			else if( month < 5 )
				totaldays += 90;
			else if( month < 6 )
				totaldays += 120;
			else if( month < 7 )
				totaldays += 151;
			else if( month < 8 )
				totaldays += 181;
			else if( month < 9 )
				totaldays += 212;
			else if( month < 10 )
				totaldays += 243;
			else if( month < 11 )
				totaldays += 273;
			else if( month < 12 )
				totaldays += 304;
			else
				totaldays += 334;
			if( month > 2 && year % 4 == 0 && ( year % 100 != 0 || year % 400 == 0 ) )
				totaldays += 1;
		}
		
		day = _day;
		totaldays += day - 1;
		
		hour = _hour;
		min = _min;
		sec = _sec;
		nsec = _nsec;

		time = (double)( ( ( totaldays * 24 + _hour ) * 60 + _min ) * 60 + sec ) + (double)nsec * 1e-9;

		if( autosummertime )
		{
			bool leapyear;
			int lastdayofmarch_daysinyear, lastdayofoctober_daysinyear, hoursinyear, days, daysperyear, newyeardayofweek, i;

			days = totaldays;
			year = 1970;
			daysperyear = 0;
			i = 0;
			while( days > daysperyear )
			{
				daysperyear = 365;
				if( !( year % 4 ) && ( ( year % 100 ) || !( year % 400 ) ) )
					daysperyear++;
				if( days > daysperyear )
				{
					i += daysperyear;
					days -= daysperyear;
					year++;
				}
			}
			newyeardayofweek = ( i + 3 ) % 7;

			leapyear = !( year % 4 ) && ( ( year % 100 ) || !( year % 400 ) );
			lastdayofmarch_daysinyear = 31 + 28 + leapyear + 31 - 1;
			lastdayofoctober_daysinyear = lastdayofmarch_daysinyear + 30 + 31 + 30 + 31 + 31 + 30 + 31;
			hoursinyear = ( totaldays - daysbeginningofyear ) * 24 + hour;
			if( hoursinyear > 1 + ( lastdayofmarch_daysinyear - ( ( 1 + ( ( lastdayofmarch_daysinyear + newyeardayofweek ) % 7 ) ) % 7 ) ) * 24
				&& hoursinyear < 3 + ( lastdayofoctober_daysinyear - ( ( 1 + ( ( lastdayofoctober_daysinyear + newyeardayofweek ) % 7 ) ) % 7 ) ) * 24
				)
				summertime = 1;
			else
				summertime = 0;
		}

		dayofweek = ( totaldays + 3 ) % 7;
		week = ( totaldays - daysbeginningofyear +  ( ( daysbeginningofyear + 3 ) % 7 ) ) / 7 + 1;
		
	}
	/// automatically sets formatted time
	/** \param _sec seconds since the epoch \param _nsec additional nanoseconds 
		\param _timezone time zone (ours additional to greenwich main time) for which time should be formatted 
		\param _summertime additional hours to add 
		\param _autosummertime automatically calculated additional hours to add, _summertime value will be ignored */
	void set( lint _sec, lint _nsec, int _timezone = TIMEFCLASS_DEF_TIMEZONE, int _summertime = 0, bool autosummertime = true )
	{
		lint totaldays, days;
		int daysperyear, dayspermonth, i, newyeardayofweek, hoursinyear, lastdayofmarch_daysinyear, lastdayofoctober_daysinyear;
		bool leapyear;
		
		time = (double)_sec + (double)_nsec * 1e-9;
		
		timezone = _timezone;
		summertime = 0;
		if( !autosummertime )
			summertime = _summertime;
		
		nsec = _nsec;
		
		totaldays = _sec + ( timezone + summertime ) * 3600;
		
		sec = totaldays % 60;
		totaldays /= 60;
		min = totaldays % 60;
		totaldays /= 60;
		hour = totaldays % 24;
		totaldays /= 24;
		
		dayofweek = ( totaldays + 3 ) % 7;
		
		days = totaldays;
		year = 1970;
		daysperyear = 0;
		i = 0;
		while( days > daysperyear )
		{
			daysperyear = 365;
			if( !( year % 4 ) && ( ( year % 100 ) || !( year % 400 ) ) )
				daysperyear++;
			if( days > daysperyear )
			{
				i += daysperyear;
				days -= daysperyear;
				year++;
			}
		}
		newyeardayofweek = ( i + 3 ) % 7;
		
		leapyear = !( year % 4 ) && ( ( year % 100 ) || !( year % 400 ) );
		
		if( autosummertime )
		{
			// Sommerzeit vom letzten Sonntag im März bis zum letzten Sonntag im Oktober
			lastdayofmarch_daysinyear = 31 + 28 + leapyear + 31 - 1;
			lastdayofoctober_daysinyear = lastdayofmarch_daysinyear + 30 + 31 + 30 + 31 + 31 + 30 + 31;
			hoursinyear = days * 24 + hour;
			// printf( "\r\n" );
			// printf( "lastdayof.._daysinyear %i %i\r\n", lastdayofmarch_daysinyear, lastdayofoctober_daysinyear );
			// printf( "newyeardayofweek %i\r\n", ( newyeardayofweek ) % 7 );
			// printf( "lastdayof.._dayofweek %i %i\r\n", ( lastdayofmarch_daysinyear + newyeardayofweek ) % 7, ( lastdayofoctober_daysinyear + newyeardayofweek ) % 7 );
			// printf( "timechange_daysinyear %i %i\r\n", ( lastdayofmarch_daysinyear - ( ( 1 + ( ( lastdayofmarch_daysinyear + newyeardayofweek ) % 7 ) ) % 7 ) ), 
			// printf( "daysinyear %i\r\n", totaldays );
			if( hoursinyear > 1 + ( lastdayofmarch_daysinyear - ( ( 1 + ( ( lastdayofmarch_daysinyear + newyeardayofweek ) % 7 ) ) % 7 ) ) * 24
				&& hoursinyear < 3 + ( lastdayofoctober_daysinyear - ( ( 1 + ( ( lastdayofoctober_daysinyear + newyeardayofweek ) % 7 ) ) % 7 ) ) * 24
				)
			{
				// printf( "autosummertime!!\r\n" );
				set( _sec, _nsec, _timezone, 1, false );
				return;
			}
		}
		/*
		6 ->  0
		5 -> -6
		4 -> -5
		3 -> -4
		2 -> -3
		1 -> -2
		0 -> -1
		*/
		
		week = ( days + ( ( totaldays - days + 3 ) % 7 ) ) / 7 + 1;
		
		month = 1;
		dayspermonth = 31;
		while( !( days < dayspermonth ) )
		{
			days -= dayspermonth;
			month++;
			switch( month )
			{
				case 2:
					dayspermonth = 28 + leapyear;
					break;
				case 3:
					dayspermonth = 31;
					break;
				case 4:
					dayspermonth = 30;
					break;
				case 5:
					dayspermonth = 31;
					break;
				case 6:
					dayspermonth = 30;
					break;
				case 7:
					dayspermonth = 31;
					break;
				case 8:
					dayspermonth = 31;
					break;
				case 9:
					dayspermonth = 30;
					break;
				case 10:
					dayspermonth = 31;
					break;
				case 11:
					dayspermonth = 30;
					break;
				case 12:
					dayspermonth = 31;
					break;
			}
		}
		day = days + 1;
		
	}
	/// automatically sets formatted time
	/** \param _time seconds since the epoch 
		\param _timezone time zone (ours additional to greenwich main time) for which time should be formatted */
	void set( double _time, int _timezone = TIMEFCLASS_DEF_TIMEZONE, int _summertime = 0, bool autosummertime = true )
	{
		set( (lint)_time, (lint)( ( _time - (double)( (lint)_time ) ) * 1e9 ), _timezone, _summertime, autosummertime );
		time = _time;
	}
	/// writes formatted time to a string
	/** \param buf string where to write to \param type formatting type (0...2) \return number of characters added to string */
	/*
	int print( char *buf, int type = 2 )
	{
		int j;
		
		j = 0;
		switch( type )
		{
			case 0:
				j += int2str( buf+j, year, 10, 4 );
				j += str2str( buf+j, "-" );
				j += int2str( buf+j, month, 10, 2 );
				j += str2str( buf+j, "-" );
				j += int2str( buf+j, day, 10, 2 );
				j += str2str( buf+j, " (" );
				j += str2str( buf+j, getsdow( dayofweek, type ) );
				j += str2str( buf+j, ", week " );
				j += int2str( buf+j, week, 10, 2 );
				j += str2str( buf+j, ") " );
				j += int2str( buf+j, hour, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, min, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, sec, 10, 2 );
				j += str2str( buf+j, "." );
				j += int2str( buf+j, nsec/1000000, 10, 3 );
				break;
			case 1:
			case 2:
			default:
				if( type % 2 )
					j += int2str( buf+j, year, 10, 4 );
				else
					j += int2str( buf+j, year%100, 10, 2 );
				j += str2str( buf+j, "-" );
				j += int2str( buf+j, month, 10, 2 );
				j += str2str( buf+j, "-" );
				j += int2str( buf+j, day, 10, 2 );
				j += str2str( buf+j, " (" );
				j += str2str( buf+j, getsdow( dayofweek, type ) );
				j += str2str( buf+j, ") " );
				j += int2str( buf+j, hour, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, min, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, sec, 10, 2 );
				j += str2str( buf+j, "." );
				j += int2str( buf+j, nsec/10000000, 10, 2 );
				break;
			case 3:
			case 4:
				if( type % 2 )
					j += int2str( buf+j, year, 10, 4 );
				else
					j += int2str( buf+j, year%100, 10, 2 );
				j += str2str( buf+j, "-" );
				j += int2str( buf+j, month, 10, 2 );
				j += str2str( buf+j, "-" );
				j += int2str( buf+j, day, 10, 2 );
				j += str2str( buf+j, "(" );
				j += str2str( buf+j, getsdow( dayofweek, type-2 ) );
				j += str2str( buf+j, ")" );
				j += int2str( buf+j, hour, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, min, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, sec, 10, 2 );
				j += str2str( buf+j, "." );
				j += int2str( buf+j, nsec/10000000, 10, 2 );
				break;
			case 9:
				j += str2str( buf+j, getsmon( month, 1 ) );
				j += str2str( buf+j, " " );
				j += int2str( buf+j, day, 10, 2 );
				j += str2str( buf+j, " (" );
				j += str2str( buf+j, getsdow( dayofweek, 2 ) );
				j += str2str( buf+j, ") " );
				j += int2str( buf+j, hour, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, min, 10, 2 );
				j += str2str( buf+j, ":" );
				j += int2str( buf+j, sec, 10, 2 );
				break;
		}
		return( j );
	}
	*/
	
	template <typename buf_t> uint printf( buf_t *buf, uint buflen, const buf_t *formatedstring )
	{
		if( !time )
			set( getseconds() );
		return( printfc( buf, buflen, formatedstring ) );
	}
	/** \param buf where to write to \param formatedstring string to write. '%' is followed by a special character, where integers and ',' are allowed between.
		first integer sets length or type of substitution of special character. special characters are: \n
		%: % character\n
		y: year\n
		m: month\n
		d: day\n
		h: hour\n
		i: minute\n
		s: second\n
		o: day of week as string\n
		n: month as string\n
		w: week \n
		f: fraction of second\n
		\return number of characters written excluding terminating null character
	*/
	template <typename buf_t> uint printfc( buf_t *buf, uint buflen, const buf_t *formatedstring ) const
	{
		uint bufpos = 0;
		if( buf )
		{
			if( formatedstring )
			{
				constexpr uint maxnumargs = 1;
				int args[maxnumargs] = { 0 };
				bool argsset[maxnumargs] = { 0 };
				uint formatpos = 0;
				while( buf_t c = formatedstring[formatpos++] )
				{
					if( c == '%' )
					{
						uint numargs = 0;
						uint argbeg = formatpos;
						for( ; ( c = formatedstring[formatpos] ); formatpos++ )
						{
							if( ( c < '0' || c > '9' ) && c != '-' && c != '*' )
							{
								if( formatpos > argbeg && !( formatpos - argbeg == 1 && formatedstring[argbeg] == '*' ) )
								{
									int iarg = str2int( formatedstring, argbeg, formatpos );
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
						switch( c )
						{
							case '%':
								if( bufpos < buflen )
									buf[bufpos++] = '%';
								break;
							case 'y':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                    args[0] < 4 ? year % 100 : year,
								                    args[0], args[0] );
								break;
							}
							case 'm':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                    month, args[0], args[0] );
								break;
							}
							case 'd':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                  day, args[0], args[0] );
								break;
							}
							case 'h':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                  hour, args[0], args[0] );
								break;
							}
							case 'i':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                  min, args[0], args[0] );
								break;
							}
							case 's':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                  sec, args[0], args[0] );
								break;
							}
							case 'o':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += str2strn( buf + bufpos, buflen - bufpos,
								                    getsdow( dayofweek, args[0] ) );
								break;
							}
							case 'n':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += str2strn( buf + bufpos, buflen - bufpos,
								                    getsmon( month, args[0] ) );
								break;
							}
							case 'w':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                  week, args[0], args[0] );
								break;
							}
							case 'f':
							{
								int defargs[] = { 2 };
								static_assert( lenof(defargs) <= lenof(args) );
								for( uint i = 0; i < lenof(defargs); i++ )
									if( !argsset[i] || i >= numargs )
										args[i] = defargs[i];
								int div = 1000000000;
								for( int k = 0; k < args[0]; k++ )
									div /= 10;
								bufpos += i2strn( buf + bufpos, buflen - bufpos,
								                  div ? nsec/div : 0, args[0], args[0] );
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

	const timefclass &setbuilttime()
	{
		int _month;
		
		if( d_strlen( __DATE__ ) == 11 && d_strlen( __TIME__ ) == 8 )
		{
			_month = 0;
			if( d_strcmp( __DATE__, "Jan", 3 ) )
				_month = 1;
			else if( d_strcmp( __DATE__, "Feb", 3 ) )
				_month = 2;
			else if( d_strcmp( __DATE__, "Mar", 3 ) )
				_month = 3;
			else if( d_strcmp( __DATE__, "Apr", 3 ) )
				_month = 4;
			else if( d_strcmp( __DATE__, "May", 3 ) )
				_month = 5;
			else if( d_strcmp( __DATE__, "Jun", 3 ) )
				_month = 6;
			else if( d_strcmp( __DATE__, "Jul", 3 ) )
				_month = 7;
			else if( d_strcmp( __DATE__, "Aug", 3 ) )
				_month = 8;
			else if( d_strcmp( __DATE__, "Sep", 3 ) )
				_month = 9;
			else if( d_strcmp( __DATE__, "Oct", 3 ) )
				_month = 10;
			else if( d_strcmp( __DATE__, "Nov", 3 ) )
				_month = 11;
			else if( d_strcmp( __DATE__, "Dec", 3 ) )
				_month = 12;
			set( str2int( __DATE__, 7, 7+4 ), _month, __DATE__[4]==' ' ? str2int( __DATE__, 5, 5+1 ) : str2int( __DATE__, 4, 4+2 ), str2int( __TIME__, 0, 0+2 ), str2int( __TIME__, 3, 3+2 ), str2int( __TIME__, 6, 6+2 ),
					  0, TIMEFCLASS_DEF_TIMEZONE, 0, true );
		}
		return( *this );
	}
};

uint print_time( char * buf, uint buflen, double time )
{
	timefclass timef;
	timef.set( time );
	return( timef.printf( buf, buflen, "%04y-%02m-%02d(%1o)%02h:%02i:%02s.%03f" ) );
}



#endif // DB_TIME_INCLUDED
