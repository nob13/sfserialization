#include <sfserialization/autoreflect.h>
#include <vector>
#include <string>

struct Collection {
	std::vector<Collection> subs;
	std::string myString;
	int myInt;
	float myFloat;
	int add1;
	int add2;
	int add3;
	int add4;
	int add5;
	SF_AUTOREFLECT_SD;
	
	void generateData (int depth) {
		myInt = 18;
		myFloat = 3.14156f;
		myString  = "A rather complex string\n";
		add1 = 1;
		add2 = 2;
		add3 = 3;
		add4 = 4;
		add5 = 5;
		
		if (depth <= 0) return;
		subs.resize (5);
		for (int i = 0; i< 5; i++) {
			subs[i].generateData(depth - 1);
		}
	}
	
	long size() const {
		long result = 9; // have 9 member elements
		for (std::vector<Collection>::const_iterator i = subs.begin(); i != subs.end(); i++) {
			result+=i->size();
		}
		return result;
	}
	
	
};

struct TestObject {
	Collection c;
	SF_AUTOREFLECT_SD;
	
	void generateData (int  depth) {
		c.generateData (depth);
	}
	
	long size() const {
		return c.size();
	}
};

