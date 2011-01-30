#include <sfserialization/Serialization.h>
#include <sfserialization/Deserialization.h> 

struct Foo {
	int myInt; 
	std::string myString; 
	
	void serialize (sf::Serialization & s) const {
		s ("myInt", myInt);
		s ("myString", myString);
	}
	
	bool deserialize (const sf::Deserialization & d) {
		bool suc = true;
		suc = d ("myInt", myInt) && suc;
		suc = d ("myString", myString) && suc;
		return suc;
	}
};

/// Is derived from Foo
struct Bar : public Foo {
	float myFloat;
	
	void serialize (sf::Serialization & s) const {
		Foo::serialize (s); // calling base class
		s ("myFloat", myFloat);
	}
	
	bool deserialize (const sf::Deserialization & d) {
		bool suc = Foo::deserialize (d); // calling base class
		suc = d ("myFloat", myFloat) && suc;
		return suc;
	}
	
};

// Test:
#include <iostream>
#include <assert.h>
int main (int argc, char * argv[]){
	Bar bar;
	bar.myFloat = 3.2; bar.myInt = 7; bar.myString = "Hello";
	
	// Serializing:
	std::cout << "JSON: " << sf::toJSON (bar) << std::endl;
	
	// Deserializing:
	std::string jsonCode = 
		"{\"myInt\":2, \"myFloat\":3.14159, \"myString\":\"Hello World\"}";	
	std::cout << "Deserializing      " << jsonCode << std::endl;
	bool suc = sf::fromJSON (jsonCode, bar);
	assert (suc);
	std::cout << "Serialized again: " << sf::toJSON (bar) << std::endl;	
	return 0;
}

