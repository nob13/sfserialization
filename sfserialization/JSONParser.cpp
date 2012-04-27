#include "JSONParser.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

namespace sf {
namespace json {

static bool emptyChar (const char c) {
	return (c == ' ' || c == '\n' || c == '\t' || c == '\f');
}

/// Scans for a string
/// sbegin will point to the beginning of the string (a '"'), slength will be the length (including "..")
static bool parseString (const char * text, int maxLength, int * slength){
	bool escaped = false;	// escape sequence
	bool began   = false;	// the string began yet
	for (int i = 0; i < maxLength; i++) {
		char c = text[i];
		if (!began) {
			if (c != '"') return false;

			// we have an '"'
			began = true;
			continue;
		}
		if (c == '"' && !escaped) {
			// end of string
			*slength = i + 1;
			return true;
		}
		escaped = false;

		if (c == '\\') {
			escaped = true;
		}
	}
	return false; // region of the string ended
}

/// scans for begin and end of a object structure; obegin and olength will include the {..} braces
bool scanObject (const char * text, int maxLength, int * olength) {
	bool inString   = false;
	bool escaped    = false; // if we are in a string and a '\' comes
	int objectDepth = 0;
	bool began = false;
	for (int i = 0; i < maxLength; i++){
		char c = text[i];

		if (!began){
			if (c != '{') return false;
			began = true;
			objectDepth = 1;
			continue;
		}

		if (c == '"') {
			if (!inString) {
				inString = true;
			}
			else if (!escaped) {
				inString = false;
			}
		}
		escaped = false;
		if (c == '\\' && inString) { escaped = true; }

		if (!inString) {
			if (c == '{') objectDepth++;
			if (c == '}') objectDepth--;
			if (objectDepth == 0){
				// end of object
				*olength = i + 1;
				return true;
			}
		}

	}
	return false; // no end found
}

/// Scans for an array
static bool scanArray (const char * text, int maxLength, int * alength){
	bool began = false;
	int arrayDepth = 0;
	bool inString = false;
	bool escaped = false;
	int i = 0;
	for (; i < maxLength; i++){
		char c = text[i];
		if (!began){
			if (c != '[') {
				return false;
			}
			began = true;
			arrayDepth = 1;
			continue;
		}

		if (c == '"') {
			if (!inString) {
				inString = true;
			}
			else if (!escaped) {
				inString = false;
			}
		}
		escaped = false;
		if (c == '\\' && inString) { escaped = true; }

		if (!inString){
			if (c == '[') arrayDepth++;
			if (c == ']') arrayDepth--;
			if (arrayDepth == 0){
				*alength = i + 1;
				return true;
			}
		}
	}
	return false;
}

/// Skips empty characters, text[position] will point to the first unempty
/// Returns false on EOF
static bool skipEmpty (const char * text, int begin, int maxLength, int * position){
	int i = begin;
	for (; i != maxLength; i++) {
		if (!emptyChar(text[i])) break;
	}
	if (i == maxLength) return false;
	*position = i;
	return true;
}


/// Scans for a character and returns its position
/// only jumps on empty characters. If a wrong character is found it returns false
static bool awaitCharacter (const char * text, int begin, int maxLength, char toAwait, int * position) {
	skipEmpty (text, begin, maxLength, position);
	if (*position == maxLength) return false;
	return (text[*position] == toAwait);
}

/// searches for an chracter, in contrast to awaitCharacter other characters maybe in place
static bool searchCharacter (const char * text, int begin, int maxLength, char toAwait, int * position){
	int i = begin;
	for (; i != maxLength; i++){
		if (text[i] == toAwait) break;
	}
	if (i == maxLength) return false;
	*position = i;
	return true;
}

/// searches for the next empty character
static bool searchEmpty (const char * text, int begin, int maxLength, int * position){
	int i = begin;
	for (; i != maxLength; i++){
		if (emptyChar (text[i])) break;
	}
	if (i == maxLength) return false;
	*position = i;
	return true;
}

/// Scans the text and compares with 'compare'
static bool nextCompare (const char * text, int maxLength, const char * compare, int * length) {
	int i = 0;
	const char * c = compare;
	for (; i < maxLength; i++) {
		if (*c == 0) {
			*length = i;
			return true;
		}
		if (text[i] != *c) return false;
		c++;
	}
	return false; // end of line
}

/// scans for a floating point number
/// Also accepts multiple '0'
static bool parseNumber (const char * text, int maxLength, int * length, bool * isFloatingPoint){
	bool sign     = false; // number is negative
	bool began    = false; // the number began
	bool hadSign  = false; // we had a sign (+ or -)
	bool hadPoint = false;
	bool hadE     = false;
	int i = 0;
	for (; i < maxLength; i++){
		char c = text[i];

		if (c == '+' || c == '-'){
			if (!hadSign) { hadSign = true; sign = (c == '-'); }
			else return false; // multiple signs
		} else
		if (c == '.') {
			if (!hadPoint) hadPoint = true;
			else return false; // multiple points
		} else
		if (c == 'e' || c == 'E') {
			if (!hadE) { hadE = true; hadSign = false; } // after e there may be a sign
			else return false; // multiple e's
		} else
		if (c >= '0' && c <= '9'){
			began = true;
			continue;
		} else if (emptyChar (c) || c == ';' || c == ',' || c == ']' || c == '}') {
			break;
		} else {
			return false; // invalid character
		}
	}
	*isFloatingPoint = (hadE || hadPoint);
	*length = i;
	return began;
}

bool Value::fetch (std::string & string, bool doDecoding, bool * decodedSuccessfull) const {
	if (mType == StringType){

		if (!doDecoding) {
			string.resize(mLength - 2);
			memcpy ((char*)string.c_str(), mData + 1, mLength - 2);
			return true;
		}
		if (decodedSuccessfull) *decodedSuccessfull = true;
		bool escaped = false;
		string.clear();
		string.reserve (mLength - 1);
		for (int i = 1; i < mLength - 1; i++){
			char c = mData[i];
			if (escaped) {
				if (c == '\"') string.append(1, '\"');
				else if (c == '\\') string.append(1, '\\');
				else if (c == 'b')  string.append(1, '\b');
				else if (c == 'f')  string.append(1, '\f');
				else if (c == 'n')  string.append(1, '\n');
				else if (c == 'r')  string.append(1, '\r');
				else if (c == 't')  string.append(1, '\t');
				else {
					*decodedSuccessfull = false;
				}
				escaped = false;
			} else {
				if (c == '\\') escaped = true;
				else string.append (1, c);
			}
		}
		return true;
	}
	return false;
}

bool Value::fetch (int64_t & data) const {
	if (mType == IntType) {
		data = iData;
		return true;
	}
	return false;
}

bool Value::fetch (double & data) const {
	if (mType == FloatType) {
		data = fData;
		return true;
	}
	if (mType == IntType) {
		data = iData;
		return true;
	}
	return false;
}

bool Value::fetch (bool & data) const {
	if (mType == BoolType) {
		data = bData;
		return true;
	}
	return false;
}

bool Value::fetch (Object & parser) const {
	if (mType != ObjectType) return false;
	parser.init (mData, mLength);
	return !parser.error();
}

bool Value::fetch (Array & array) const {
	if (mType != ArrayType) return false;
	array.parse (mData, mLength);
	return !array.error();
}

bool Value::numFetch (int64_t & data) const {
	if(mType == IntType) {
		data = iData;
		return true;
	}
	if(mType == FloatType) {
		data = (int64_t) fData;
		return true;
	}
	if (mType == StringType) {
		int len;
		bool isFloat;
		if (mLength >= 2 && parseNumber (mData + 1, mLength - 2, &len, &isFloat)){
			errno = 0; // maybe tainted (it has happend!)
			if (isFloat){
				double f = strtod (mData + 1, NULL);
				if (!errno) {
					data = (int64_t) f;
				}
			} else {
				int64_t i = strtol (mData+ 1, NULL, 10);
				if (!errno) {
					data = i;
				}
			}
			return !errno; // error on conversion
		}
		return false; // not a number
	}
	return false;
}

bool Value::numFetch (double & data) const {
	if(mType == IntType) {
		data = (double) iData;
		return true;
	}
	if(mType == FloatType) {
		data = fData;
		return true;
	}
	if (mType == StringType) {
		int len;
		bool isFloat;
		if (mLength >= 2 && parseNumber (mData + 1, mLength - 1, &len, &isFloat)){
			errno = 0; // maybe tainted (it has happend!)
			if (isFloat){
				double f = strtod (mData + 1, NULL);
				if (!errno){
					data = f;
				}
			} else {
				int64_t i = strtol (mData+ 1, NULL, 10);
				if (!errno) {
					data = (double) i;
				}
			}
			return !errno; // error on conversion
		}
		return false; // not a number
	}
	return false;
}

bool Value::parse (const char * text, int maxLength){
	if (maxLength <= 0) return false;

	mData = text;


	if (*mData == '"' && parseString (mData, maxLength, &mLength)){
		mType = StringType;
		return true;
	}
	if (*mData == '{' && scanObject (mData, maxLength, &mLength)){
		mType = ObjectType;
		return true;
	}
	if (*mData == '[' && scanArray (mData, maxLength, &mLength)){
		mType = ArrayType;
		return true;
	}
	if (*mData == 't' && nextCompare (mData, maxLength, "true", &mLength)){
		mType = BoolType;
		bData = true;
		return true;
	}
	if (*mData == 'f' && nextCompare (mData, maxLength, "false", &mLength)){
		mType = BoolType;
		bData = false;
		return true;
	}
	if (*mData == 'n' && nextCompare (mData, maxLength, "null", &mLength)){
		mType = NullType;
		return true;
	}
	bool isFloat;
	if (parseNumber (mData, maxLength, &mLength, &isFloat)){
		errno = 0; // maybe tainted (it has happend!)
		if (isFloat){
			fData = strtod (mData, NULL);
			mType = FloatType;
		} else {
			iData = strtol (mData, NULL, 10);
			mType = IntType;
		}
		return !errno;
	}
	return false; // could not recognize object

}

const Value & Array::get (int id) const {
	static Value invalid;
	if (id >= 0 && id < (int) mValues.size()) return mValues[id];
	return invalid;
}

void Array::parse (const char * data, int length) {
	mError = false;
	mValues.clear();
	mValues.reserve(16);
	assert (length > 0);

	mData = data;
	mDataLength = length;
	int i = 0;
	int old = -1;

	if (mData[i] != '[') goto ErrorCase;
	i++;
	while (i < length){
		assert (i > old);
		old = i;
		if (!skipEmpty (mData, i, mDataLength, &i)) goto ErrorCase;

		// can also end after ',' and directly after beginning
		if (mData[i] == ']') goto SuccessCase;
		
		Value v;
		bool result = v.parse (mData + i, length - i);
		if (!result) goto ErrorCase;
		i+=v.mLength;

		mValues.push_back (v);
		int pos = 0;
		bool next = awaitCharacter (mData, i, mDataLength, ',', &pos);
		if (!next) {
			bool final = awaitCharacter (mData, i, mDataLength, ']', &pos);
			if (!final) goto ErrorCase;
			goto SuccessCase;
		}
		i = pos + 1;
	}
	// early end?
	goto ErrorCase;

	SuccessCase:
	return;

	ErrorCase:
	mValues.clear ();
	mError = true;
	return;
}

Object::Object (const Object & object) {
	operator=(object);
}

Object & Object::operator= (const Object & object) {
	mData         = object.mData;
	mLength       = object.mLength;
	mEntries      = object.mEntries;
	mError        = object.mError;
	mErrorMessage = object.mErrorMessage;
	mEntries.updateNextEntries ();
	return *this;
}

void Object::init (const char * data, std::string & command, int length){
	if (length < 0) length = strlen (data);

	int cmdBegin, cmdEnd;
	if (!skipEmpty (data, 0, length, &cmdBegin)) { mError = true; return; }
	char c = data[cmdBegin];

	if (c == '{') { command = ""; init (data + cmdBegin, length - cmdBegin); return; } // no command found

	int emptyPos;
	int bracePos;
	bool foundEmpty = searchEmpty (data, cmdBegin, length, &emptyPos);
	bool foundBrace = searchCharacter (data, cmdBegin, length, '{', &bracePos);
	if (!foundBrace) { mError = true; return; } // there is no JSON without '{'
	if (foundEmpty) {
		cmdEnd = std::min (emptyPos, bracePos);
	} else {
		cmdEnd = bracePos;
	}

	command.resize (cmdEnd - cmdBegin);
	memcpy ((char*) command.c_str(), data + cmdBegin, cmdEnd - cmdBegin);

	init (data + cmdEnd, length - cmdEnd);
}

void Object::parse () {
	enum State { 
		Begin,				// Waiting for beginning { 
		AwaitingKey, 		// Waiting for beginning key or for ending structure
		AwaitingValue,		// Waiting for a value
	};
	
	State state = Begin;
	
	Entry entry;
	
	char c = 0;
	int  i = 0;
	int  last = -1;
	while (i < mLength) {
		assert (i > last);
		last = i;
		c = mData[i];
		if (!skipEmpty (mData, i, mLength, &i)) goto ErrorCase;
		c = mData[i];
		
		switch (state) {
			case Begin: {
				if (c == '{') { state = AwaitingKey; i++; continue; }
				goto ErrorCase;
			}
			break;
			case AwaitingKey: {
				if (c == ',') { i++; continue; }
				if (c == '}') { i++; goto EndCheck; }
				int length;
				if (parseString (mData + i, mLength - i, &length)) {
					assert (length >= 2); // must be ".."
					entry.mName = mData + i + 1;
					entry.mNameLength = length - 2; // without doublequotes
					i+=length;
					// wait for ':'
					int pos = 0;
					bool foundSep = awaitCharacter (mData, i, mLength, ':', &pos);
					if (!foundSep) goto ErrorCase;
					i = pos + 1;
					state = AwaitingValue;
				} else goto ErrorCase;
			}
			break;
			case AwaitingValue : {
				bool found = entry.mValue.parse (mData + i, mLength - i);
				i+=entry.mValue.mLength;
				if (!found) goto ErrorCase;
				mEntries.insertEntry (entry);
				state = AwaitingKey;
				// i++; // position shall be already on the next one
			}
			break;			
		}
	}
	
	mError = true;
	mErrorMessage = "Sudden End";
	return;
	
	EndCheck:
	for (;i < mLength; i++){
		c = mData[i];
		if (!emptyChar (c)) goto ErrorCase; 
	}
	// All Ok
	mEntries.updateNextEntries();
	return;
	
	ErrorCase:
	mError = true;
	char buffer [256];
	snprintf (buffer, 256, "Error parsing around %c in state %d at position %d\n", c, state, i);
	mErrorMessage = buffer;
	mEntries.clear();
}

void Object::streamOut (std::ostream & stream) const {
	if (mError) stream << "[Err:]";
	char * buffer = new char [mLength + 1];
	buffer[mLength] = 0;
	memcpy (buffer, mData, mLength);
	stream << buffer;
	delete [] buffer;
}

Value parse (const char * data, std::string & command, int length) {
	if (length < 0) length = strlen (data);

	int cmdBegin, cmdEnd;
	if (!skipEmpty (data, 0, length, &cmdBegin)) {
		return Value ();
	}
	char c = data[cmdBegin];

	if (c == '{') {
		command = ""; // cmd not found
		return parse (data + cmdBegin, length - cmdBegin);
	}

	int emptyPos;
	int bracePos;
	bool foundEmpty = searchEmpty (data, cmdBegin, length, &emptyPos);
	bool foundBrace = searchCharacter (data, cmdBegin, length, '{', &bracePos);
	if (!foundBrace) {
		return Value(); // there is no JSON Object without '{'
	}

	if (foundEmpty) {
		cmdEnd = std::min (emptyPos, bracePos);
	} else {
		cmdEnd = bracePos;
	}

	command.resize (cmdEnd - cmdBegin);
	memcpy ((char*) command.c_str(), data + cmdBegin, cmdEnd - cmdBegin);

	return parse (data + bracePos, length - bracePos);
}

Value parse (const char * data, int length) {
	if (length < 0) length = strlen (data);
	Value v;
	v.parse (data, length);
	return v;
}

}
}

