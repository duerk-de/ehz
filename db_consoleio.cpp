__GIT_SYMLINKS__
target=../_inc/db_consoleio.cpp
target_resolved=/dibom/src/c/_inc/db_consoleio.cpp
kind=file
sha256=f45add398cc989e6acf1f70254ab5c1172346aa5c26d81021842056e9f52073f
size=4530
created=2026-11-16T20:49:12.733878
--END-HEADER--
#ifndef DB_CONSOLEIO_INCLUDED
#define DB_CONSOLEIO_INCLUDED

#include "db_mem.cpp"
#include "db_str.cpp"
#include "db_defines.cpp"
#include "db_time.cpp"


#ifndef LINUX
	
	#include <windows.h>
	#include <cstdio>
	#include <conio.h>
	
	void out( char* buf )
	{
		CharToOem( buf, buf );
		printf( "%s", buf );
	}
	void out( char const* buf )
	{
		static const uint max = 512 - 1;
		// static char bbuf[max+1];
		char *bbuf;
		if( buf && ( bbuf = new char[max+1] ) )
		{
			uint l;
			do {
				for( l=0; buf[l] && l<max; l++ );
				if( buf[l] )
				{
					scpy( bbuf, buf, l-1 );
					CharToOem( bbuf, bbuf );
					printf( "%s", bbuf );
					buf += l-1;
				} else {
					CharToOem( buf, bbuf );
					printf( "%s", bbuf );
					buf += l;
				}
			} while( *buf );
			dropa( bbuf );
		}
	}
	
#else

	#include <stdio.h>
	#include <termios.h>
	#include <fcntl.h>
	
	void outp( char const* buf, int maxstrlen, int strlen = -1, FILE * std = stdout )
	{
		fwrite( buf, sizeof(char), strlen < 0 ? d_strlen( buf, maxstrlen ) : strlen, std );
		fflush( std );
	}
	
	template <uint maxstrlen>
	void out( char const (&buf)[maxstrlen], int strlen = -1, FILE * std = stdout )
	{
		fwrite( buf, sizeof(char), strlen < 0 ? d_strlen( buf, maxstrlen ) : strlen, std );
		fflush( std );
	}
	
	template <uint buflen = 8 * (1<<10)> uint d_fprint( char const* formatedstring, ... )
	{
		char buf[buflen];
		va_list arguments;
		va_start( arguments, formatedstring );
		uint bufpos = d_fstrn_internal( buf, lenof(buf), formatedstring, arguments );
		va_end( arguments );
		out( buf, bufpos, stdout );
		return( bufpos );
	}
	
	template <uint buflen = 8 * (1<<10)> uint d_fprint_err( char const* formatedstring, ... )
	{
		char buf[buflen];
		va_list arguments;
		va_start( arguments, formatedstring );
		uint bufpos = d_fstrn_internal( buf, lenof(buf), formatedstring, arguments );
		va_end( arguments );
		out( buf, bufpos, stderr );
		return( bufpos );
	}
	
	bool kbhit()
	{
		struct termios oldt, newt;
		int ch, oldf;
		tcgetattr( STDIN_FILENO, &oldt );
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt );
		oldf = fcntl( STDIN_FILENO, F_GETFL, 0 );
		fcntl( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK );
		ch = getchar();
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
		fcntl( STDIN_FILENO, F_SETFL, oldf );
		if( ch != EOF )
		{
			ungetc( ch, stdin );
			return( true );
		} else {
			return( false );
		}
	}
	inline int getch()
	{
		return( getchar() );
	}
	
#endif


int buildheader( char *buf, uint buflen, char const* name,
                 char const* years, bool linebreaks = true )
{
	timefclass btime;
	uint bufpos = 0;
	if( linebreaks )
		bufpos += str2strn( buf + bufpos, buflen - bufpos, "\n" );
	bufpos += str2strn( buf + bufpos, buflen - bufpos, name );
	bufpos += str2strn( buf + bufpos, buflen - bufpos, " - (c)" );
	bufpos += str2strn( buf + bufpos, buflen - bufpos, years );
	bufpos += str2strn( buf + bufpos, buflen - bufpos, " d.b.company  -  built " );
	
	btime.setbuilttime();
	btime.min = btime.min / 5 * 5;
	bufpos += btime.printfc( buf + bufpos, buflen - bufpos, "%y-%m-%d %h-%i" );
	
	#ifndef __OPTIMIZE__
	bufpos += str2strn( buf + bufpos, buflen - bufpos, " (not optimized)" );
	#endif

	if( linebreaks )
		bufpos += str2strn( buf + bufpos, buflen - bufpos, "\n\n" );
	
	return( bufpos );
}

void printheader( char const* name, char const* years, bool linebreaks = true )
{
	char buf[256];
	buildheader( buf, lenof(buf), name, years, linebreaks );
	out( buf );
}

bool ui_stop( char *buf, uint buflen, char *_key = null )
{
	char key;
	bool stop;
	double d;
	
	stop = false;
	key = 0;
	while( kbhit() )
	{
		key = getch();
		if( key == 27 )
		{
			out( "\r" );
			for( d=0.05, key=0; !key && d<10.0; d+=0.1 )
			{
				if( buf )
				{
					uint bufpos = 0;
					bufpos += str2strn( buf + bufpos, buflen - bufpos, "\rpress 'q' to quit ... " );
					bufpos += time2strn( buf + bufpos, buflen - bufpos, 10.0-d, 2 );
					bufpos += str2strn( buf + bufpos, buflen - bufpos, "   " );
					outp( buf, buflen, bufpos );
				}
				sleeps( 0.1 );
				while( kbhit() )
				{
					key = getch();
					if( key == 'q' )
						stop = true;
				}
			}
			if( buf )
				out( "\r                               \r" );
		}
	}
	if( _key )
		*_key = key;
	return( stop );
}


#endif // DB_CONSOLEIO_INCLUDED
