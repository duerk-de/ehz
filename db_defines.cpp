__GIT_SYMLINKS__
target=../_inc/db_defines.cpp
target_resolved=/dibom/src/c/_inc/db_defines.cpp
kind=file
sha256=9c9f4b5ed0acd23131823b8e3bb754d6802277bf44a72be791fa68c306e127da
size=425
created=2026-11-15T10:29:04.642698
--END-HEADER--
#if defined(__GCCE__) && defined(__WINSCW__)
	#define SYMBIAN
#else
	#ifdef _WIN32
		#define WINDOWS
		#define _WIN32_WINNT 0x0501
	#else
		#define LINUX
	#endif
#endif

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned long long ulint;
typedef long long lint;
typedef wchar_t wchar;
#define null 0
// typedef nullptr null
