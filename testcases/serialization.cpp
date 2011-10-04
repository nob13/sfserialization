#include "test.h"
#include <sfserialization/Serialization.h>
#include <sfserialization/Deserialization.h>
#include <sfserialization/JSONParser.h>

/*
 * Tests the JSON serializing / deserializing routines
 */

enum MyEnumType {
	Alpha,
	Beta,
	Gamma
};

const char * toString (MyEnumType x) {
	const char * values [] = { "Alpha", "Beta", "Gamma" };
	const int l = sizeof (values) / sizeof (const char*);
	const int i = (int) x;
	if (i < 0 || i > l) {
		tassert (false && "Invalid value");
		return "invalid";
	}
	return values[i];
}

bool fromString (const std::string & s, MyEnumType & x){
	if (s == "Alpha") { x = Alpha; return true;  }
	if (s == "Beta") { x = Beta;  return true; }
	if (s == "Gamma") { x = Gamma; return true; }
	return false;
}

struct SubType {
	SubType () : a(-5), b(false), c (true), d(Beta) {}

	void serialize (sf::Serialization & s) const {
		s ("a", a);
		s ("b", b);
		s ("c", c);
		s ("d", d);
	}

	bool deserialize (const sf::Deserialization & des) {
		// newer API
		bool succ = true;
		succ = succ && des ("a", a);
		succ = succ && des ("b", b);
		succ = succ && des ("c", c);
		succ = succ && des ("d", d);
		return succ;
	}

	float a;
	bool b;
	bool c;
	MyEnumType d;
};

struct Externizable : SubType {
	Externizable () : e1(5), e2(-7) {
		text = "anotherone with backslashes \\ s \\ and \"quotes\"";
		array.push_back (334);
		array.push_back (435);
		array.push_back (213);
	}

	void serialize (sf::Serialization & s) const {
		s ("int-value", e1);
		s ("int-value2", e2);
		s ("array", array);
		s ("textelement", text);
		SubType::serialize (s); 
	}

	bool deserialize (const sf::Deserialization & d) {
		bool ret = true;
		ret = ret && d ("int-value", e1);
		ret = ret && d ("int-value2", e2);
		SubType::deserialize (d);
		return ret;
	}

	int e1, e2;
	std::string text;
	std::vector<int> array;
};

bool plainSerialization () {
	// Serialization / Deserialization of plain types.
	{
		int x = 5;
		std::string s = sf::toJSON (x);
		int y = 0;
		bool suc = sf::fromJSON (s, y);
		printf ("Sux=%d x=%d y=%d\n", suc, x, y);
		tassert (suc && (x == y));
	}
	{
		std::vector<int> x; x.push_back (3); x.push_back (4);
		std::string s = sf::toJSON(x);
		std::vector<int> y;
		bool suc = sf::fromJSON (s, y);
		tassert (suc && (x == y));
	}
	{
		std::string x = "Its a tricky\nString\"";
		std::string s = sf::toJSON (x);
		std::string y;
		bool suc = sf::fromJSON (s, y);
		tassert (suc && (x == y));
	}
	return true;
}

int main (int argc, char * argv[]){
	Externizable e;
	SubType st;
	std::string s = sf::toJSON (e);
	std::string t = sf::toJSON (st);
	printf ("Serialized %s, Subtype %s\n", s.c_str(), t.c_str());
	
	
	printf ("Deserializing\n");
	bool ret = e.deserialize (s);
	tassert (ret, "Shall deserialize");

	RUN (plainSerialization());

	return 0;
}
