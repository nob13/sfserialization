#include "reflect_struct.h"
#include <stdio.h>

#include <sfserialization/Serialization.h>
#include <sfserialization/Deserialization.h>
#include "test.h"

bool testCyclus () {
	my::Derived d;
	d.a = 5;
	d.b = -3.5;
	d.c = "Hi you are \"cool\"";
	d.d.push_back (3);
	d.d.push_back (4);
	d.keys["Alpha"] = "1";
	d.keys["Beta"]  = "complex\n \t, yes";
	
	std::string json = sf::toJSON (d);
	
	// Deserialize it again
	my::Derived d2;
	bool suc = sf::fromJSON (json, d2);
	if (!suc){
		fprintf (stderr, "Deserilization failed\n");
		return false;
	}
	suc = (d2 == d);
	if (!suc) {
		fprintf (stderr, "Serialization does not match deserialization\n");
		fprintf (stderr, "Serialized:   %s\n", sf::toJSON (d).c_str());
		fprintf (stderr, "Deserialized: %s\n", sf::toJSON (d2).c_str());
		return false;
	}
	
	// Multiple deserilazation shall lead to the same object
	// (Members must be deleted upon deserialization)
	suc = sf::fromJSON (json, d2);
	if (!suc) {
		fprintf (stderr, "Deserialization failed (2)\n");
		return false;
	}
	suc = (d2 == d);
	if (!suc) {
		fprintf (stderr, "Serialization does not match deserialization (2)\n");
		fprintf (stderr, "Serialized:   %s\n", sf::toJSON (d).c_str());
		fprintf (stderr, "Deserialized: %s\n", sf::toJSON (d2).c_str());
		return false;
	}
	
	// Deserialization of a clean object must kill all members
	suc = sf::fromJSON ("{}", d2);
	if (!suc) {
		fprintf (stderr, "Deserialization of empty type failed\n");
		return false;
	}
	suc = d2.isDefault();
	if (!suc) {
		fprintf (stderr, "Did not deserialized an empty JSON object\n");
		fprintf (stderr, "Still in there: %s\n", sf::toJSON (d2).c_str());
	}
	return true;
}

bool testPrivateAvoidance1 () {
	my::WithPrivate1 p (5,5);
	std::string json = sf::toJSON (p);
	bool suc = (json == "{\"a\":5}");
	if (!suc) {
		fprintf (stderr, "Autoreflect did not avoid serialization of private members (1): %s\n", json.c_str());
		return false;
	}
	my::WithPrivate1 p2 (5,3);
	suc = sf::fromJSON (json, p2);
	if (!suc) {
		fprintf (stderr, "Deserialization failed\n");
		return false;
	}
	if (p2.getB() != 3) {
		fprintf (stderr, "Deserialization deserialized private member!");
		return false;
	}
	return true;
}

bool testPrivateAvoidance2 () {
	my::WithPrivate2 p (5,5);
	std::string json = sf::toJSON (p);
	bool suc = (json == "{\"a\":5}");
	if (!suc) {
		fprintf (stderr, "Autoreflect did not avoid serialization of private members (2): %s\n", json.c_str());
		return false;
	}
	my::WithPrivate2 p2 (5,3);
	suc = sf::fromJSON (json, p2);
	if (!suc) {
		fprintf (stderr, "Deserialization failed\n");
		return false;
	}
	if (p2.getB() != 3) {
		fprintf (stderr, "Deserialization deserialized private member!");
		return false;
	}
	return true;
}

bool testMapWithIntKey() {
	my::MapWithIntKey testMap;
	testMap.values[3] = "Hello World";
	std::string json = sf::toJSON(testMap);
	std::string expected = "{\"values\":{\"3\":\"Hello World\"}}";
	bool suc = (json == expected);
	if (!suc) {
		fprintf(stderr, "Could not serialize map with int as key, got %s instead of %s\n", json.c_str(), expected.c_str());
		return false;
	}
	my::MapWithIntKey back;
	suc = sf::fromJSON(json, back);
	if (!suc) {
		fprintf(stderr, "Could not serialize map with int key back\n");
		return false;
	}
	// Error Handling on invalid JSON.
	std::string jsonWithBadKey = "{\"values\":{\"A\":\"Hello World\"}}"; // "A" doesn't fit into int
	my::MapWithIntKey back2;
	suc = sf::fromJSON(jsonWithBadKey, back2);
	if (suc) {
		fprintf(stderr, "Should detect bad deserialization, got %s from %s", sf::toJSON(back2).c_str(), jsonWithBadKey.c_str());
		return false;
	}
	return true;
}

int main (int argc, char * argv[]) {
	my::Base     b;
	my::Derived  d;
	my::Compound c;
	printf ("Base:     %s\n", sf::toJSON (b).c_str());
	printf ("Derived:  %s\n", sf::toJSON (d).c_str());
	printf ("Compound: %s\n", sf::toJSON (c).c_str());
	
	printf ("Base     As Cmd: %s\n", sf::toJSONCmd (b).c_str());
	printf ("Derived  As Cmd: %s\n", sf::toJSONCmd (d).c_str());
	printf ("Compound As Cmd: %s\n", sf::toJSONCmd (c).c_str());
	
#ifdef __GNUC__
	// IsDefault
	printf ("SFINAE Test: %d\n", sf::hasIsDefault<my::Base>::value);
	printf ("SFINAE Test: %d\n", sf::hasIsDefault<int>::value);
#endif
	printf ("IsDefault for Base  : %d\n", sf::isDefault (b));
	printf ("IsDefault for 0     : %d\n", sf::isDefault (0));
	printf ("IsDefault for 1     : %d\n", sf::isDefault (1));
	printf ("IsDefalt for vector : %d\n", sf::isDefault (std::vector<int>()));
	
	// Check compressed, must be empty as types where not changed
	tassert (sf::toJSONEx (b, sf::COMPRESS) == "{}");
	tassert (sf::toJSONEx (d, sf::COMPRESS) == "{}");
	tassert (sf::toJSONEx (c, sf::COMPRESS) == "{}");
	
	// Full serialization and deserialization
	RUN (testCyclus());
	RUN (testPrivateAvoidance1());
	RUN (testPrivateAvoidance2());
	RUN (testMapWithIntKey());
	
	return 0;
}
