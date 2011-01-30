#include <sfserialization/Serialization.h>
#include <sfserialization/Deserialization.h>

#include "performance.h"
#include "test.h"
#include <stdio.h>

void testrun (int depth, int iterations) {
	TestObject object;
	object.generateData (depth);
	long size  = object.size();
	long bytes = 0;
	double t0 = microtime();
	for (int i = 0; i < iterations; i++) {
		// printf ("Iteration: %d\n", i);
		std::string json = sf::toJSON (object);
		bytes +=json.size();
		TestObject object2;
		bool suc = sf::fromJSON (json, object2);
		tassert (suc);
	}
	double t1 = microtime ();
	double t = t1 - t0;
	long elements = size * iterations;
	printf ("Testrun with depth %d and %d iterations  (single element size: %ld)\n", depth, iterations, size);
	printf ("  Parsed %ld elements (%ld bytes) in %f seconds\n", elements, bytes, t);
	printf ("  1000 Elements per second: %f\n", elements / t / 1000);
	printf ("  MiB per second:           %f\n", bytes / t / (1024 * 1024));
	printf ("\n");
}

int main (int argc, char * argv[]){
	testrun (7, 1);
	testrun (5, 20);
	testrun (1, 10000);
	testrun (0, 100000);
	return 0;
}

