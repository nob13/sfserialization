#include "reflect_enum.h"
#include <stdio.h>
#include "test.h"
#include <sfserialization/isdefault.h>

template <class E>
bool checkFromString (const std::string & s, E e){
	E x;
	bool suc = fromString (s.c_str(), x);
	if (suc) {
		return x == e;
	}
	return false;
}

template <class X>
bool testToAndFrom (X x) {
	X back;
	bool suc = fromString (toString (x), back);
	if (!suc) return false;
	return (x == back);
}

bool testBitEnum () {
	if (!testToAndFrom (Bit0)) return false;
	if (!testToAndFrom (Bit1)) return false;
	if (!testToAndFrom (Bit2)) return false;
	if (!testToAndFrom (Bit3)) return false;
	if (!testToAndFrom (Bit4)) return false;
	if (!testToAndFrom (Bit5)) return false;
	if (!testToAndFrom (Bit6)) return false;
	if (!testToAndFrom (Bit7)) return false;
	if (!testToAndFrom (Bit8)) return false;
	if (!testToAndFrom (Bit9)) return false;
	if (!testToAndFrom (Bit10)) return false;
	if (!testToAndFrom (Bit11)) return false;
	if (!testToAndFrom (Bit12)) return false;
	if (!testToAndFrom (Bit13)) return false;
	if (!testToAndFrom (Bit14)) return false;
	if (!testToAndFrom (Bit15)) return false;
	return true;
}

int main (int argc, char * argv[]){
	printf ("%s %s %s %s\n", toString (Alpha), toString (Beta), toString (Gamma), toString (Delta));
	
	// Correct generation
	tassert (toString (Alpha) == std::string("Alpha"));
	tassert (toString (Beta) == std::string("Beta"));
	tassert (toString (Gamma) == std::string("Gamma"));
	tassert (toString (Delta) == std::string("Delta"));
	
	tassert (checkFromString ("Alpha", Alpha));
	tassert (checkFromString ("Beta", Beta));
	tassert (checkFromString ("Gamma", Gamma));
	tassert (checkFromString ("Delta", Delta));
	tassert (!checkFromString ("Invalid", Alpha));

	tassert (toString (A) == std::string ("A"));
	tassert (toString (B) == std::string ("B"));
	tassert (checkFromString ("A", A));
	tassert (checkFromString ("B", B));
	tassert (!checkFromString ("Invalid", A));

	// User values must know their default state (Alpha == 0)
	tassert (sf::isDefault (Alpha));

	tassert (sf::isDefault (TestEmptyEnum()));

	tassert (sf::isDefault (Bla::A));

	tassert (testBitEnum());

	return 0;
}
