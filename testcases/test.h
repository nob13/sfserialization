#include <stdio.h>
#include <stdlib.h>

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
