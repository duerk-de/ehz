#ifndef BACKTRACE_INCLUDED
#define BACKTRACE_INCLUDED

#include "db_consoleio.cpp"

#include <string>
#include <execinfo.h>
#include <cxxabi.h>

namespace bt
{
	constexpr int P_BLK  = 30;
	constexpr int P_RED  = 31;
	constexpr int P_GRN  = 32;
	constexpr int P_YEL  = 33;
	constexpr int P_BLU  = 34;
	constexpr int P_MGT  = 35;
	constexpr int P_CYN  = 36;
	constexpr int P_LGRY = 37;
	constexpr int P_DGRY = 90;
	constexpr int P_LRED = 91;
	constexpr int P_LGRN = 92;
	constexpr int P_LYEL = 93;
	constexpr int P_LBLU = 94;
	constexpr int P_LMGT = 95;
	constexpr int P_LCYN = 96;
	constexpr int P_STD  = 39;
	constexpr int P_BOLD =  1;
	constexpr int P_DIM  =  2;
	constexpr int P_ULIN =  3;
	constexpr int P_BLNK =  5;
	constexpr int P_RVRS =  7;
	constexpr int P_NONE =  0;
	constexpr char P_BEG[] = "\x1b[";
	constexpr char P_SEP[] = ";";
	constexpr char P_END[] = "m";

	uint lg10(uint num)
	{
		uint lg = 0;
		while( num )
		{
			lg++;
			num /= 10;
		}
		return( lg );
	}
	
	bool find_file_function( char const* frame, uint & filepos, uint & filelen,
	                         uint & fncpos, uint & fnclen )
	{
		uint l = 1 << 8;
		char const* p = frame;
		if( !*p )
			return( false );
		filepos = ( p - frame );
		++l;
		while( --l && *p && *p != '(' )
			p++;
		filelen = ( p - frame ) - filepos;
		if( !*p )
			return( false );
		p++;
		fncpos = ( p - frame );
		++l;
		while( --l && *p && *p != ')' && *p != '+' )
			p++;
		fnclen = ( p - frame ) - fncpos;
		return( true );
	}
	
	template <uint buflen>
	bool get_file_function( char * frame, char (&buf)[buflen],
	                        char const* & file, uint & filelen,
	                        char const* & fnc, uint & fnclen )
	{
		uint filepos;
		uint fncpos;
		if( !find_file_function( frame, filepos, filelen, fncpos, fnclen ) )
			return( false );
		file = frame + filepos;
		frame[fncpos+fnclen] = '\0';
		fnc = frame + fncpos;

		size_t len = buflen;
		int stat = 1;
		char const* dfnc = abi::__cxa_demangle( fnc, buf, &len, &stat );
		if( dfnc && !stat )
		{
			fnc = dfnc;
			fnclen = d_strlen( dfnc, len );
		}
		return( true );
	}
	

	void print( char const* errFile = nullptr, int errLine = -1, int errFrame = 1 )
	{
		void * faddrs[64] = { 0 };
		int num_frames = backtrace( faddrs, lenof( faddrs ) );
		if( num_frames > (int)lenof( faddrs ) ) 
			num_frames = (int)lenof( faddrs );
		int lg_num_frames = lg10( num_frames );
		char ** frames = backtrace_symbols( faddrs, num_frames );
		if( frames && ( num_frames > 1 ) )
		{
			uint maxfilelen = 0;
			uint maxfnclen = 0;
			for( int i = 0; i < num_frames; i++ )
			{
				if( frames[i] )
				{
					char const* file = "?";
					uint filelen = 1;
					char const* fnc = "?";
					uint fnclen = 1;
					char buf[1024] = { 0 };
					get_file_function( frames[i], buf, file, filelen, fnc, fnclen );
					if( filelen > maxfilelen )
						maxfilelen = filelen;
					if( fnclen > maxfnclen )
						maxfnclen = fnclen;
				}
			}
			for( int i = 0; i < num_frames; i++ )
			{
				if( frames[i] )
				{
					char const* file = "?";
					uint filelen = 1;
					char const* fnc = "?";
					uint fnclen = 1;
					char buf[1024] = { 0 };
					get_file_function( frames[i], buf, file, filelen, fnc, fnclen );
					
					d_fprint( "%s%i%s%i%s" "%*i: %*,*s: %*,*s" "%s%i%s" ,
							  P_BEG, P_BOLD, P_SEP, i > 2 ? P_BLU : P_CYN, P_END,
							  lg_num_frames, num_frames - i,
							  filelen, maxfilelen, file,
							  fnclen, maxfnclen, fnc,
							  P_BEG, P_NONE, P_END );
					if( errFile && i == errFrame )
						d_fprint( ": " "%s%i%s%i%s" "%s:%i" "%s%i%s",
						          P_BEG, P_BOLD, P_SEP, P_YEL, P_END,
						          errFile, errLine,
						          P_BEG, P_NONE, P_END );
					d_fprint( "\n"  );
						
				}
			}
			free( (void*)frames );
		}
	}
};

#endif // BACKTRACE_INCLUDED

