#pragma once
#include "types.h"
#include "isdefault.h"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_enum.hpp>

namespace sf {
class Serialization;

/** This class helps serializing objects into JSON.
	It is the opposite of sf::Deserialization.

	How to use:
	@verbatim
	void MyCoolStruct::serialize (sf::Serialization & s) const {
		MyCoolStruct::serialize (s); // Do this with all your base classes
		s ("fieldA", fieldA);        // do this with all fields.
	}
	@endverbatim

	Afterwards you can serialize your objects of type MyCoolStruct
	with

	@verbatim
	MyCoolStruct s;
	std::string jsonCode                    = sf::toJSON (s);
	@endverbatim
*/
class Serialization {
public:
	/**
	 * Initializes JSON Serialization
	 * @param target         where the JSON code will go
	 * @param compress       compress the output (only serialize objects, which are not default values)
	 * @param indent         do some nice indentation
	 */
	inline Serialization (std::string & target, bool compress = false, bool indent = false) :
		mTarget (target), 
		mNeedComma (false), 
		mIndent (indent),
		mIndentation (0),
		mCompress (compress),
		mFillness (0) { }
	inline ~Serialization (){ flush (); }
	
	/// Flush output buffer to target string
	void flush ();
	
	/// Gives a hint how big the serialization will probably be (used for reserving data)
	/// This will alloc some bytes more (do not call it multiple times)
	void sizeHint (size_t data);

public:

	///@ manual Building JSON Structure (Commas are done automatically)
	///@{
	
	/// Insert a control character (must be '[', '{', '}', '])
	void insertControlChar (char c);
	/// Insert a key name (afterwards it will wait for a value)S
	void insertKey         (const char * key);
	/// Insert command name (should be done only before doing everything else)
	/// Note: not JSON standard compatible
	void insertCommand     (const char * cmd);
	/// Insert a generic value
	void insertValue       (const char * value);
	/// Insert a string value (does also \n-Handling etc.)
	void insertStringValue (const char * stringValue);
	
	///@}

	/// Serializes a field with given key name and Value
	/// (Same like put)
	template <class T> void operator () (const char * key, const T & value){
		if (!mCompress || !isDefault(value)){
			insertKey (key);
			serialize (*this, value);
		}
	}

	/// Serializes a field with given key name and Value
	/// (Same like operator() but without default handling)
	template <class T> void put (const char * key, const T & value){
		insertKey (key);
		serialize (value);
	}
	
	
private:
	void cacheAppend (const char * s);
	void cacheAppend (const char c);
	
	/// Adds a string to the cache with escape symbols and begin/end " .. "
	void addString (const char * c);
	std::string & mTarget;
	bool mNeedComma;		///< Next one needs a comma
	
	bool mIndent;			///< Does new lines for each field and some nice indentation
	int  mIndentation;		///< current indentation level
	
	bool mCompress;			///< Only serialize values which are different to isDefault()
	
	int mFillness;			
	char mCache[512];	
};

/// Converts a given object to simple JSON
/// Does NOT compress nor add a command string (regular use)
template <class T> std::string toJSON (const T & obj){
	std::string target;
	Serialization serialization (target);
	serialize (serialization, obj); // koenig lookup
	serialization.flush();
	return target;
}

/// Converts a given object to a JSON command; does compress
template <class T> std::string toJSONCmd (const T & obj){
	std::string target;
	Serialization serialization (target, true, false);
	serialization.insertCommand (T::getCmdName());
	serialize (serialization, obj); // koenig lookup
	serialization.flush();
	return target;
}

enum SerializationFlags {
	COMPRESS = 0x1,		///< Compress (omits values which are default values)
	INDENT   = 0x4,		///< Indent the output (for better human readability)
};

/// Converts a given object to JSON, you can control wether to compress, to start with name (command mode) or to indent.
/// flags - flags ORED of SerializationFlags
template <class T> std::string toJSONEx (const T & obj, int flags) {
	std::string target;
	bool compress = flags & COMPRESS;
	bool indent   = flags & INDENT;
	Serialization serialization (target, compress, indent);
	serialize (serialization, obj); // koenig lookup
	serialization.flush();
	return target;
}

// Serializer for plain types

void serialize (Serialization & s, int32_t data);
void serialize (Serialization & s, int64_t data);
void serialize (Serialization & s, uint32_t data);
void serialize (Serialization & s, uint64_t data);
void serialize (Serialization & s, float data);
void serialize (Serialization & s, double data);
void serialize (Serialization & s, bool data);
void serialize (Serialization & s, const std::string& data);
void serialize (Serialization & s, const char* data);

/// Serialize method for sets
template <class T> static void serialize (sf::Serialization & s, const std::set<T> & container) {
	s.insertControlChar ('[');
	for (typename std::set<T>::const_iterator i = container.begin(); i != container.end(); i++){
		serialize (s, *i);
	}
	s.insertControlChar (']');
}

/// Serialize methods for vectors
template <class T> static void serialize (sf::Serialization &s, const std::vector<T> & container) {
	s.insertControlChar ('[');
	for (typename std::vector<T>::const_iterator i = container.begin(); i != container.end(); i++){
		serialize (s, *i);
	}
	s.insertControlChar (']');
}

/// Serialize method for maps
template <class T> void serialize (Serialization & s, const std::map<std::string, T> & data) {
	s.insertControlChar('{');
	for (typename std::map<std::string, T>::const_iterator i = data.begin(); i != data.end(); i++) {
		s.insertKey (i->first.c_str());
		serialize (s, i->second);
	}
	s.insertControlChar('}');
}

/// Serialize method for a pair
template <typename A, typename B> void serialize (Serialization & s, const std::pair<A,B> & obj){
	s.insertControlChar ('{');
	s.insertKey ("1st"); 
	serialize (s, obj.first);
	s.insertKey ("2nd"); 
	serialize (s, obj.second);
	s.insertControlChar ('}');
}

/// Serialize an enum
template <typename T>
 typename boost::enable_if< boost::is_enum<T>, void>::type
 serialize (Serialization & s, const T & t){
	const char * str = toString (t);
	s.insertStringValue (str);
}

/// General serialize function for objects
template <typename T>
 typename boost::disable_if< boost::is_enum<T>, void>::type
serialize (sf::Serialization & s, const T & obj){
	s.insertControlChar ('{');
	obj.serialize (s);
	s.insertControlChar ('}');
}

/// General serialize function for objects
template <class T> static void serialize (sf::Serialization & s, T * const obj) {
	s.insertControlChar ('{');
	obj->serialize (s);
	s.insertControlChar('}');
}

}
