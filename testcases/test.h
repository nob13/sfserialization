#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <sfserialization/winsupport.h>
#include <Windows.h>
#endif

// A timer with microsecond exactness
double microtime (){
#ifdef WIN32
	FILETIME time;
	GetSystemTimeAsFileTime (&time);
	int64_t full = 0;
	full |= time.dwHighDateTime;
	full <<= 32;
	full |= time.dwLowDateTime;
	// is in 100nano-seconds intervals...
	static int64_t first = full;
	int64_t use = (full - first);
	return (use / 10000000.0);
#else
	struct timeval t;
	gettimeofday (&t, 0);
	return t.tv_sec + t.tv_usec / 1000000.0;
#endif
}

#define RUN(X) \
	if (!X) { \
		fprintf (stderr, "%s failed\n", #X); \
		return 1; \
	} else { \
		printf ("%s successfull\n", #X);\
	}\

/// An assertion command which doesn't get optimized away by NDEBUG
inline void tassert (bool x, const char * msg = 0){
	if (!x) {
		if (!msg)
			fprintf (stderr, "Assert failed!\n");
		else
			fprintf (stderr, "Assert failed: %s\n", msg);
		::abort ();
	}
}
