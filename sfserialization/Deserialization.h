#pragma once
#include "types.h"
#include "JSONParser.h"
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_enum.hpp>

namespace sf {
class Deserialization;

#ifdef __GNUC__
// SFINAE test whether there is an deserialize method
template <typename T>
class hasDeserialize
{
    typedef char one;
    typedef long two;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    template <typename C> static one test( decltype(&C::deserialize) ) ;
#else
    template <typename C> static one test( typeof(&C::deserialize) ) ;
#endif
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// forward declaration
template <typename T>
 typename boost::enable_if_c< hasDeserialize<T>::value, bool>::type deserialize (const json::Value & v, T & obj);
#endif
#ifdef _MSC_VER
template <typename T>
 typename boost::disable_if< boost::is_enum<T>, bool>::type deserialize (const json::Value & v, T & e);
#endif


/// Reads int32 value from the item
/// @return true on success
inline bool deserialize (const json::Value & v, int32_t & i){
	int64_t x;
	bool suc = v.numFetch (x);
	if (suc) {
		i = x;
	}
	return suc;
}

/// Reads int64 value from the item
/// @return true on success
inline bool deserialize (const json::Value & v, int64_t & i){
	return v.numFetch(i);
}

/// Reads float value from the item
/// @return true on success
inline bool deserialize (const json::Value & v, float & f){
	double x;
	bool suc = v.numFetch(x);
	if (suc) {
		f = x;
	}
	return suc;
}

/// Reads double value from the item
/// @return true on success
inline bool deserialize (const json::Value & v, double & d){
	return v.numFetch(d);
}

/// Reads std::string value from item
/// @return true on success
inline bool deserialize (const json::Value & v, std::string & s){
	return v.fetch(s, true);
}

/// Reads boolean value from item
/// @return true on success
inline bool deserialize (const json::Value & v, bool & b) {
	return v.fetch(b);
}

/// Reads an enum value (with fromString method)
template <typename T>
 typename boost::enable_if< boost::is_enum<T>, bool>::type deserialize (const json::Value & v, T & e){
	std::string s;
	bool suc = v.fetch(s);
	if (!suc) return false;
	return fromString (s.c_str(), e);
}

/// Reads a std::set out of a json Array
template <class T> bool deserialize (const json::Value & v, std::set<T> & set){
	json::Array a;
	if (!v.fetch(a)) return false;
	set.clear ();
	for (int i = 0; i < a.count(); i++){
		T x;
		json::Value sub = a.get(i);
		if (!deserialize (sub, x)) return false;
		set.insert (x);
	}
	return true;
}

/// Reads a std::vector out of a json Array
template <class T> bool deserialize (const json::Value & v, std::vector<T> & vector){
	json::Array a;
	if (!v.fetch(a)) return false;
	vector.clear ();
	for (int i = 0; i < a.count(); i++){
		T x;
		json::Value sub = a.get(i);
		if (!deserialize (sub, x)) return false;
		vector.push_back (x);
	}
	return true;
}

/// Fetches all keys into a map
template <class T> bool deserialize (const json::Value & v, std::map<std::string, T> & dst) {
	json::Object o;
	if (!v.fetch(o)) return false;
	dst.clear ();
	const json::Entry * e = o.first ();
	while (e) {
		T t;
		if (!deserialize (e->value(), t)) return false;
		dst[e->name()] = t;
		e = e->next ();
	}
	return true;
}

// Fetches a pair
template <class A, class B> bool deserialize (const json::Value & v, std::pair<A,B> & dst) {
	json::Object o;
	if (!v.fetch(o)) return false;
	return deserialize (o.get("1st"), dst.first) && deserialize (o.get("2nd"), dst.second);
}

/**
 Provides Deserialization of JSON based objects
 with an API similar to class Serialization.

 How to use:
	@verbatim
	bool MyCoolStruct::deserialize (const sf::Derialization & d) {
		bool retValue = BaseClass::deserialize (d);
		retValue = s ("fieldA", fieldA) && retValue; // do this with all your member variables
		return retValue;                             // so we know wether deserialization was successfull, you can do additional checks for consistency.
	}
	@endverbatim

Afterwards you can deserialize like this:

	@verbatim
	std::string someJSONCode = "MyCoolStruct {\"fieldA\":5}";

	std::string cmd;
	sf::Deserialization d (someJSONCode, cmd); // cmd will now be "MyCoolStruct"
	MyCoolStruct s;
	bool success = s.deserialize (d);
	// check success etc.
	@endverbatim

You can also use the sf::fromJSON command:
    @verbatim
    MyCoolStruct s;
    std::string json = "{\"fieldA\":5}";
    bool suc = sf::fromJSON (json, s);
	// check success
    @endverbatim

*/
class Deserialization {
public:
	Deserialization ();
	/// Initializes with a string; it will make a copy of it
	Deserialization (const std::string & s);
	/// Initializes with a string, gives you access to the command addition, makes a copy of the data
	Deserialization (const std::string & s, std::string & cmd);
	/// Initializes Deserialization, does NOT make a copy
	Deserialization (const ByteArrayBase & array);
	/// Initializes Deserialization with a ByteArrayBase, does NOT make a copy (useful for deserializing and interpreting commands)
	Deserialization (const ByteArrayBase & array, std::string & cmd);
	
	/// Initializes with a ready parsed json-Object; note it wont make a copy
	/// so keep the data available
	Deserialization (const sf::json::Object& o);

	/// Access one key and saves it in value
	/// If not found it does either return an error
	/// or use the default value for a given type (if setUseDefaultValue is set to true)
	/// @return true on success
	template <class T> bool operator() (const char * key, T & value) const {
		json::Value v = mObject.get(key);
		if (v.valid()){
			return deserialize (v, value);
		}
		value = T();
		return true;
	}

	/// Access one key and saves it in value. If key is not found, use an default value
	/// @return true if key is not found and default value was used or key was found and from right type.
	template <class T> bool operator() (const char * key, T & value, const T & defaultValue) const {
		json::Value v = mObject.get(key);
		if (v.valid()){
			return deserialize (v, value);
		}
		value = defaultValue;
		return true;
	}

	/// Accesss one key and saves it value, if not found is the default value for the given type
	/// @depreciated
	template <class T> bool defaultGetFetch (const char * key, T & value) const {
		return operator()(key, value);
	}
	
	/// Fetches all keys into a map
	template <class T> bool allFetch (std::map<std::string, T> & dst) const {
		dst.clear ();
		const json::Entry * e = mObject.first ();
		while (e) {
			T t;
			if (!deserialize (e->value(), t)) return false;
			dst[e->name()] = t;
			e = e->next();
		}
		return true;
	}

	/// Deserialization has an error (during parsing, not during getting!)
	bool error () const { return mObject.error(); }

	/// Streams out Deserialization to std::ostream
	void streamOut (std::ostream & stream) const;
private:
	std::string  mText;
	json::Object mObject;
};

/// Deserializes a object from JSON code
/// @return true on success
template <class T> bool fromJSON (const std::string & txt, T & dst){
	json::Value v = json::parse(txt.c_str(), txt.length());
	return deserialize (v, dst);
}

/// Deserializes a object from JSON code
/// @return true on success
template <class T> bool fromJSON (const ByteArrayBase & data, T & dst){
	json::Value v = json::parse(&data.front(), data.size());
	return deserialize (v, dst);
}

#ifdef __GNUC__
/// Reads a regualar object value (with deserialize method)
template <typename T>
 typename boost::enable_if_c< hasDeserialize<T>::value, bool>::type deserialize (const json::Value & v, T & obj){
	json::Object o;
	if (!v.fetch (o)){
		return false;
	}
	Deserialization d (o);
	if (d.error()){
		return false;
	}
	return obj.deserialize (d);
}
#endif

#ifdef _MSC_VER
template<class T> typename boost::disable_if< boost::is_enum<T>, bool>::type deserialize (const json::Value & v, T & obj){
	json::Object o;
	if (!v.fetch (o)) return false;
	Deserialization d (o);
	if (d.error()) return false;
	return obj.deserialize (d);
 }
#endif

}

/// Outstream operator
std::ostream & operator<<  (std::ostream & stream, const sf::Deserialization & deserialization);


