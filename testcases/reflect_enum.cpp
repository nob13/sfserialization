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

	return 0;
}
