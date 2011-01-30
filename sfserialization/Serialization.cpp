#include "Serialization.h"

#include <stdio.h>
#include <assert.h>

namespace sf {

#ifdef WIN32
// Hack
#define snprintf _snprintf
#endif

/// Own rewrite of itoa for decimal numbers
template <class T> static void myItoa (T v, char * result, int base){
	char * p = result;
	bool neg = v < 0;
	if (neg) v = -v;
	do {
		int x = v % base;
		v /= base;
		*p = "0123456789abcdefgh" [x];
		p++;
	} while (v);
	if (neg) { *p = '-'; p++; }
	*p = '\0';
	p--;
	char * s = result;
	while (s < p){
		char t = *s;
		*s = *p;
		*p = t;
		s++;
		p--;
	}
}

void Serialization::sizeHint (size_t size){
	mTarget.reserve (mTarget.size() + size);
}

void Serialization::flush (){
	mTarget.append (mCache, mFillness);
	mFillness = 0;
}

void Serialization::insertControlChar (char c) {
	if (!(c == '{' || c == '}' || c == '[' || c == ']')){
		fprintf (stderr, "Serialization::operator<< Waited for control char");
		assert (false && "Waited for control char");
		return;
	}
	if (mNeedComma && ( c == '[' || c == '{')) { 
		cacheAppend (", "); 
		mNeedComma = false; 
	}  
	if (c == '{') mIndentation++;	
	if (c == '}') mIndentation--;
	
	if (c == '}' || c == ']') mNeedComma = true;
	
	if (c == '}' && mIndent){
		cacheAppend ("\n");
		for (int i = 0; i < mIndentation; i++){
			cacheAppend ("  ");
		}
	}
	
	cacheAppend (c);
}

void Serialization::insertKey (const char * key) {
	if (mNeedComma) cacheAppend (", ");
	
	if (mIndent){
		cacheAppend ("\n");
		for (int i = 0; i < mIndentation; i++) cacheAppend ("  ");
	}
	
	addString (key);
	cacheAppend (':');
	mNeedComma = false;
}

void Serialization::insertCommand (const char * cmd) {
	cacheAppend (cmd);
	cacheAppend (' ');
}

void Serialization::insertValue   (const char * value) {
	if (mNeedComma) cacheAppend (", ");
	cacheAppend (value);
	mNeedComma = true;
}

void Serialization::insertStringValue (const char * stringValue) {
	if (mNeedComma) cacheAppend (", ");
	addString (stringValue);
	mNeedComma = true;
}


void Serialization::cacheAppend (const char * s) { 
	const char * c = s;
	while (*c != 0){
		mCache[mFillness] = *c;
		mFillness++;
		c++;
		if (mFillness > 511) { mTarget.append (mCache, mFillness); mFillness = 0; }
	}
}
void Serialization::cacheAppend (const char c){
	mCache[mFillness] = c;
	mFillness++;
	if (mFillness > 511) { mTarget.append (mCache, mFillness); mFillness = 0; } 
}


void Serialization::addString (const char * s){
	cacheAppend ('"');
	for (const char *i = s; *i != 0; i++){
		switch (*i){
			case '\\': cacheAppend ("\\\\"); break;
			case '"': cacheAppend ("\\\""); break;
			case '\n': cacheAppend ("\\n"); break;
			case '\t': cacheAppend ("\\t"); break;
		default: cacheAppend (*i);
		}
	}
	cacheAppend ('"');
}

void serialize (Serialization & s, int32_t data) {
	char buf [16];
	myItoa (data, buf, 10);
	s.insertValue(buf);
}

void serialize (Serialization & s, int64_t data) {
	char buf [32];
	myItoa (data, buf, 10);
	s.insertValue(buf);
}

void serialize (Serialization & s, uint32_t data) {
	char buf [16];
	myItoa (data, buf, 10);
	s.insertValue (buf);
}

void serialize (Serialization & s, uint64_t data) {
	char buf [32];
	myItoa (data, buf, 10);
	s.insertValue (buf);
}

void serialize (Serialization & s, float data) {
	char buf [32];
	snprintf (buf, 32, "%.8g", data);
	s.insertValue(buf);
}

void serialize (Serialization & s, double data) {
	char buf [32];
	snprintf (buf, 32, "%.16g", data);
	s.insertValue (buf);
}

void serialize (Serialization & s, bool data) {
	if (data) s.insertValue ("true");
	else s.insertValue ("false");
}

void serialize (Serialization & s, const std::string& data) {
	s.insertStringValue (data.c_str());
}

void serialize (Serialization & s, const char* data) {
	s.insertStringValue (data);
}



}
