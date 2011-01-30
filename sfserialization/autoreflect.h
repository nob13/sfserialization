#ifndef SF_AUTOREFLECT_HEADER_GUARD
#define SF_AUTOREFLECT_HEADER_GUARD

namespace sf {
	class Serialization;
	class Deserialization;
}

// serialize and isDefault() method
#define SF_AUTOREFLECT_SERIAL \
	void serialize (sf::Serialization & s) const; \
	bool isDefault () const;

// serialize, isDefault and deserialize method
#define SF_AUTOREFLECT_SERIAL_DESERIAL \
	SF_AUTOREFLECT_SERIAL; \
	bool deserialize (const sf::Deserialization & s);

/// Get cmd name generates a command name for a type
#define SF_AUTOREFLECT_GETCMDNAME \
	static const char * getCmdName (); 

// Abbreviation for SF_AUTOREFLECT_SERIAL_DESERIAL
#define SF_AUTOREFLECT_SD \
	SF_AUTOREFLECT_SERIAL_DESERIAL

// Abbrevation for SF_AUTOREFLECT_SERIAL_DESERIAL + SF_AUTOREFLECT_GETCMDNAME
#define SF_AUTOREFLECT_SDC \
	SF_AUTOREFLECT_SERIAL_DESERIAL \
	SF_AUTOREFLECT_GETCMDNAME;

// Enum toString and fromString method
#define SF_AUTOREFLECT_ENUM_TOFROMSTRING(X)\
	const char * toString (X e);\
	bool fromString (const char* s, X & e);

// Abbreviation for SF_AUTOREFLECT_ENUM_TOFROMSTRING 
#define SF_AUTOREFLECT_ENUM(X) \
	SF_AUTOREFLECT_ENUM_TOFROMSTRING(X);

namespace sf {

typedef unsigned long HashValue;
// Hash algorithm found on http://www.cse.yorku.ca/~oz/hash.html
inline HashValue hash(const unsigned char *str) {
	HashValue hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

}

#endif
