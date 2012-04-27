#pragma once

/**
 * @file
 *  A JSON Parser for small JSON structures.
 *
 * It uses a std::vector for storing all the attributes, a std::map would be faster for large ones.
 * (It shall be easy to put in, just look into the json::Object class)
 *
 * @Note:
 * - There is no support for the @\u escape sequence.
 * - For key names there is no support for escapes at all.
 * - Decoding of strings with escape sequences (fetch (name, string, true)) may be very slow.
 * - The parser has to be only dependent on standard C++ Stuff, no boost, no other libs.
 *
 * The main class is json::Object, just feed it with your JSON code and access all elements via get() and fetch ()
 *
 * @Todo:
 * - Allow comments,
 * - Do not need quotes for the key-names (but the standard wants that)
 */

#include <string.h>
#include <string>
#include <vector>
#include <ostream>

#ifdef WIN32
#include "winsupport.h"
#else
#include <stdint.h>
#endif

namespace sf {
namespace json {

///@cond DEV

/// Type of a value
enum ValueType {
	InvalidType = 0, ObjectType, IntType, FloatType, StringType, BoolType, NullType, ArrayType
};

class Object;
class Array;

/// A JSON Value
class Value {
public:
	Value () : mData (0), mLength (0), mType(InvalidType) {}

	/// Fetches an string
	/// @param doDecoding do decoding of escape sequences
	/// @return whether type was Ok
	bool fetch (std::string & string, bool doDecoding = false, bool * decodedSuccessfull = 0) const;

	/// Fetches an long int type.
	/// @return whether type was Ok
	bool fetch (int64_t & data) const;

	/// Fetches an int type.
	/// Note: Internally long is used, you may run into trouble
	/// @return whether type was Ok
	bool fetch (int32_t & data) const { int64_t x; bool ret = fetch (x); if (ret) data = (int32_t) x; return ret; }

	/// Fetches an double type.
	/// @return whether type was Ok
	bool fetch (double & data) const;

	/// Fetches an float type.
	/// Note: Internally double is used, you may run into trouble
	/// @return whether type was Ok
	bool fetch (float & data) const { double x; bool ret = fetch (x); if (ret) data = (float) x; return ret; }

	/// Fetches an boolean type.
	/// @return whether type was Ok
	bool fetch (bool & data) const;

	/// Fetches a a sub object.
	/// @return whether type was Ok and the parser could parse the subtype.
	bool fetch (Object & parser) const;

	/// Fetches an array
	/// @return whether type was Ok and the array was successfully parsed
	bool fetch (Array & array) const;

	/// Fetches an numerical type (do not cars about int/float/string)
	/// @return whether type was Ok
	bool numFetch (int64_t & data) const;

	/// Fetches an numerical type (do not cars about int/float/string)
	/// @return whether type was Ok
	bool numFetch (double & data) const;

	/// Fetches a value only if present, if not use a default value
	template <class T> void dFetch (T & x, const T & defaultValue) const {
		bool ret = fetch (x);
		if (!ret) x = defaultValue;
	}

	/// Returns the type of a specific field, InvalidType is returned if the field was not found
	ValueType type () const { return mType; }

	/// Value is a valid type
	bool valid () const { return mType != InvalidType; }

	/// Fetches a a sub object.
	/// @return when name was found, type was Ok and the parser could parse the subtype.
	bool fetchSubObject (Object & parser) const { return fetch (parser); }

	///@cond DEV

	/// Parses a value; returns true on success
	bool parse (const char * text, int maxLength);

	/// Returns error state (means invalid type)
	bool error () const { return mType == InvalidType; }

private:
	friend class Object;
	friend class Array;
	const char * mData;		///< Position where the entry relies
	int mLength;			///< Length of the value field
	union {
		double fData;		///< Double data (if type == FloatType)
		int64_t iData;			///< Integer data (if type == IntType)
		bool bData;			///< Boolean data (if type == BoolType)
	};
	ValueType mType;
	///@endcond DEV
};

/// A stored name value pair
class Entry {
public:
	Entry () : mName (0), mNext (0) {}

	/// Returns the name of the entry
	std::string name () const { return std::string (mName, mName + mNameLength); }
	/// Returns the value of the entry.
	const Value & value () const { return mValue; }
	/// Returns the next entry, or 0 if there is no next
	const Entry* next () const { return mNext; }
private:
	friend class Object;
	const char * mName;		///< Name of the key (not 0-terminated)
	int mNameLength;		///< Length of the key
	Value mValue;
	Entry * mNext;			///< Next entry
};

/// A JSON Array Type
class Array {
public:

	Array () {
		mError = true;
	}

	/// Initializes an array
	Array (const char * array, int length) {
		parse (array, length);
	}

	/// Returns error state
	bool error () const { return mError; }

	/// Access to the entries
	const Value & get (int id) const;

	/// How many entries are in the array
	int count () const { return mValues.size(); }

	void parse (const char * array, int length);
private:
	const char * mData;
	int mDataLength;
	typedef std::vector<Value> ValueVector;
	ValueVector mValues;
	bool mError;
};


/**
 * A JSON Object (something with "{" .. "}")
 *
 * Usually the main object
 */
class Object {
public:

	///@name State
	///@{
	
	/**
	 * Creates an uninitialized parser
	 */
	Object (){
		mData = 0;
		mLength = -1;
		mError = true;
		mErrorMessage = "not initialized";
	}
	
	/**
	 * Initializes JSON Object from an existing one.
	 * @note:
	 * - it does not generate a copy of the source text.
	 * - it does copy entries and thus it is possibly slow.
	 */
	Object (const Object & object);

	/**
	 * Copy assignment.
	 * @note
	 * - It does not generate a copy of the source text.
	 * - It does copy entries and thus it is possibly slow.
	 */
	Object& operator= (const Object & object);


	/**
	 * Parsed the code in data. If length = -1 it assumes the data to be null-terminated,
	 * otherwise it is uses the given length
	 */
	Object (const char * data, int length = -1) {
		init (data, length);
	}
	
	/**
	 * Parses a null-terminated string. The string may begin with a command (arbitrary word
	 * without quotes). The command will be stored in command.
	 * @note
	 * - Object does not hold a copy of the text. It uses the given one.
	 */
	Object (const char * data, std::string & command, int length = -1){
		init (data, command, length);
	}

	/// Returns if there was an error during parsing
	bool error () const { return mError; }
	
	/// Human readable error description
	std::string errorMessage () const { return mErrorMessage; }

	/**
	 * (Re-)initializes the parser and parses the code. If length == -1 it assumes the data to be 
	 * null-terminated otherwise it uses the given length.
	 *
	 * @note
	 * - Object does not hold a copy of the text. It uses the given one.
	 */
	void init (const char * data, int length = -1){
		mEntries.clear ();
		mEntries.reserve (32);
		mData = data;
		mLength = length < 0 ? strlen (data) : length;
		mError = false;
		mErrorMessage = "";
		parse ();
	}
	
	/**
	 * (Re-)initializes the parser and parses the code. If length == -1 it assumes the data to be
	 * null-terminated otherwise it uses the given length. This variant provides support of an
	 * arbitrary command, which comes before the real object. An example is 'MyCoolCommand {"user="bla"}'
	 * (without quotes)
	 */
	void init (const char * data, std::string & command, int length = -1);

	///@}
	
	///@name Field Access
	///@{
	
	/// Fetches a value with the given key. Returns 0 if nothing found
	const Value & get (const char * name) const {
		const Entry * e = mEntries.findEntry (name);
		static Value invalidValue; // default initializes to invalid
		return e ? e->value() : invalidValue;
	}
	
	const Entry * first () const {
		return mEntries.first();
	}
	
	size_t entryCount() const {
		return mEntries.size();
	}

	///@}

	/// Streams out JSON code
	void streamOut (std::ostream & stream) const;
private:
	///@cond DEV

	/// Table of entries with search options via name
	/// Replace this table in order to use a HashMap
	struct EntryTable : private std::vector<Entry> {
		const Entry * findEntry (const char * name) const {
			EntryTable::const_iterator i;
			int l = (int) strlen (name);
			for (i = begin(); i != end(); i++){
				const Entry & e = *i;
				if (e.mNameLength != l) continue; // the name is saved with double quotes ("..")
				if (strncmp (e.mName, name, l) == 0) return &e;
			}
			return 0;
		}

		const Entry * first () const {
			return empty() ? 0 : &*begin();
		}

		void insertEntry (const Entry & e){
			push_back (e);
		}

		void updateNextEntries () {
			iterator e = end();
			for (iterator i = begin(); i != e; i++) {
				iterator j = i + 1;
				if (j != e){
					i->mNext = &(*j);
				} else {
					i->mNext = 0;
				}
			}
		}

		void reserve (size_t n){
			std::vector<Entry>::reserve (n);
		}

		void clear () {
			std::vector<Entry>::clear ();
		}
		
		size_t size () const {
			return std::vector<Entry>::size();
		}
	};
	///@endcond DEV

	const char * mData;				///< JSON code to be parsed
	int			 mLength;			///< Length of JSON code to be parsed
	
	EntryTable mEntries;	///< The entries inside the current JSON code
	
	bool mError;
	std::string mErrorMessage;
	
	/// Parses the JSON file
	void parse ();
};

/// Parses a JSON object and returns it in a json::Value
/// Note: it must be an object!
Value parse (const char * data, std::string & command, int length = -1);

/// Parses a JSON string and returns it in a json::Value
/// Note : it can be an arbitrary json code
Value parse (const char * data, int length = -1);


///@endcond DEV

}
}

/// Output operator
inline std::ostream & operator<< (std::ostream & stream, const sf::json::Object & o) { o.streamOut (stream); return stream; }
