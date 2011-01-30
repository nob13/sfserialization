#include <sfserialization/Serialization.h>
#include <sfserialization/Deserialization.h>
#include "test.h"

// Regression Bugs (bugs which do come back)

struct WithMap {
	typedef std::map<std::string, std::string> StringMap;
	WithMap () {
	}
	
	void init () {
		map["A"] = "A1";
		map["B"] = "B1";
	}
	
	StringMap map;
	
	void serialize (sf::Serialization & s) const {
		s("map", map);
	}
	
	bool deserialize (const sf::Deserialization & d) {
		return d("map", map);
	}
	
	static bool test () {
		WithMap x;
		x.init();
		std::string serialized = sf::toJSON (x);
		printf ("Serialized: >>%s<<\n", serialized.c_str());
		
		bool suc = serialized == "{\"map\":{\"A\":\"A1\", \"B\":\"B1\"}}";
		
		WithMap y;
		suc = sf::fromJSON (serialized, y) && suc;
		suc = (x.map == y.map) && suc;
		return suc;
	}
};

struct WithPair {
	typedef std::pair<std::string, std::string> StringPair;
	StringPair pp;
	
	WithPair () {
	}
	
	void init () {
		pp = std::make_pair("A", "B");
	}
	
	void serialize (sf::Serialization &  s) const {
		s ("pp", pp);
	}
	
	bool deserialize (const sf::Deserialization & d) {
		return d ("pp", pp);
	}
	
	static bool test () {
		WithPair x;
		x.init();
		std::string serialized = sf::toJSON (x);
		printf ("Serialized: >>%s<<\n", serialized.c_str());
		bool suc = serialized == "{\"pp\":{\"1st\":\"A\", \"2nd\":\"B\"}}";
		WithPair y;
		suc = sf::fromJSON (serialized, y) && suc;
		suc = (y.pp == y.pp) && suc;
		return suc;
	}
};

/// STL type with an serializeable internal type
struct SubType {
	struct A{
		A() {}
		A(int _x) { x = _x;}
		
		void serialize (sf::Serialization & s) const {
			s ("x", x);
		}
		bool deserialize (const sf::Deserialization & d){
			return d ("x", x);
		}
		bool operator== (const A & a) const {
			return x == a.x;
		}
		bool operator< (const A & a) const {
			return x < a.x;
		}
		int x;
	};
	typedef std::set <A> ASet;
	ASet set;

	void serialize (sf::Serialization & s) const {
		s ("set", set);
	}
	
	bool deserialize (const sf::Deserialization & d) {
		return d ("set", set);
	}
	
	static bool test () {
		SubType x;
		x.set.insert (5);
		x.set.insert (3);
		std::string json = sf::toJSON (x);
		SubType y;
		bool suc = sf::fromJSON (json, y);
		suc = suc && x.set == y.set;
		return suc;
	}
};

int main (int argc, char * argv[]){
	RUN (WithMap::test());
	RUN (WithPair::test());
	RUN (SubType::test());
	return 0;
}
