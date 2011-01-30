#pragma once

#include <sfserialization/autoreflect.h>
#include <vector>
#include <map>
#include <string>

namespace my {

struct Base {
	Base () : a(0), b(0) {}
	int              a;
	float            b;
	std::string      c;
	std::vector<int> d;
	bool operator== (const Base & other) const {
		return a == other.a && b == other.b && c == other.c && d == other.d;
	}
	
	SF_AUTOREFLECT_SDC;
};

struct Derived : public Base {
	std::map<std::string, std::string> keys;
	
	bool operator == (const Derived & other) const {
		return Base::operator==(other) && keys == other.keys;
	}
	
	SF_AUTOREFLECT_SDC;
};

struct Compound {
	Base     sub1;
	Derived  sub2;
	SF_AUTOREFLECT_SDC;
};

// Autoreflect may not serialize private members
struct WithPrivate1 {
	WithPrivate1 (int _a = 0, int _b = 0) : a (_a), b(_b) {}
	int a;
	SF_AUTOREFLECT_SD;
	int getB() const { return b;}
private:
	int b;
};

// Autoreflect may not serialize private members (and have to know that classes are private by default)
class WithPrivate2 {
	int b;
public:
	WithPrivate2 (int _a = 0, int _b = 0) : b (_b), a(_a) {}
	int getB() const { return b;}
	int a;
	SF_AUTOREFLECT_SD;
};


}

namespace other {

struct OtherDerived : public my::Derived {
	int x;
	SF_AUTOREFLECT_SDC;
};

}
