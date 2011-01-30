#include "Preprocessor.h"
#include <iostream>
#ifdef WIN32
#include <fstream>
#endif
#include <stdio.h>
#include <stdlib.h>

bool PreprocessorPrintingDelegate::onPreprocess (const std::string & line) {
	printf ("Preprocess: %s\n", line.c_str());
	return true;
}

bool PreprocessorPrintingDelegate::onToken (Iterator begin, Iterator end) {
	printf ("Token: %s\n", std::string(begin,end).c_str());
	return true;
}

bool PreprocessorPrintingDelegate::onLineComment (const std::string & content) {
	printf ("Line Comment: %s\n", content.c_str());
	return true;
}

bool PreprocessorPrintingDelegate::onComment (const std::string& content) {
	printf ("Comment: %s\n", content.c_str());
	return true;
}

bool PreprocessorPrintingDelegate::onStringLiteral (const std::string & content) {
	printf ("String Literal: %s\n", content.c_str());
	return true;
}

bool PreprocessorPrintingDelegate::onCharacterLiteral (char c) {
	printf ("Character Literal: %c\n", c);
	return true;
}

bool PreprocessorPrintingDelegate::onSyntactic (char c) {
	printf ("Syntactic: %c\n", c);
	return true;
}

bool PreprocessorPrintingDelegate::onEnd () {
	printf ("End\n");
	return true;
}

/// throws away empty strings at beginning
static std::string lstrip (const std::string& s){
	std::string::const_iterator i = s.begin();
	for (; i != s.end() && isspace (*i); i++){
	}
	return std::string (i, s.end());
}

Preprocessor::Preprocessor (PreprocessorDelegate * delegate){
	mLine      = 0;
	mInComment = false;
	mDelegate  = delegate;
}

Preprocessor::~Preprocessor () {
}

bool Preprocessor::loadFile (const char * filename) {
#ifdef WIN32
	std::ifstream srcFile (filename);
	if (!srcFile.is_open()) {
		fprintf (stderr, "Could not open %s\n", filename);
		return false;
	}
	std::string line;
	while (std::getline(srcFile, line)){
		bool result = pushLine(line);
		if (!result){
			int lineN = lineNum ();
			fprintf (stderr, "Error on line %d, current line %s\n", lineN, line.c_str());
			return false;
		}
	}
	return true;
#else
	// Win32 does not have gnu getline extension
	FILE * srcFile = fopen (filename, "r");
	if (!srcFile) {
		fprintf (stderr, "Could not open %s\n", filename);
		return false;
	}
	size_t nbytes = 8192;
	char * buffer = (char*) malloc (nbytes + 1);
	while (true){
		ssize_t read = getline (&buffer, &nbytes, srcFile);
		if (read == -1) break;
		bool result = pushLine (buffer);
		if (!result){
			int line = lineNum ();
			fprintf (stderr, "Error on line %d, current line %s\n", line, buffer);
			fclose (srcFile);
			return false;
		}
	}
	fclose (srcFile);
#endif
	
	bool result = pushEnd();
	if (!result){
		fprintf (stderr, "Error on end of file\n");
		return false;
	};
	
	return true;
}


bool Preprocessor::pushLine (const std::string & line){
	mLine++;
	
	std::string s;
	if (!mInComment){
		s = lstrip (line);
		if (s.empty())   return true;  // empty line
		if (s[0] == '#') {
			return mDelegate->onPreprocess (s);
		}
	} else s = line;

	
	// Current Position
	Iterator x  = s.begin();
	// Current start
	Iterator a  = s.begin();
	bool inStringLiteral = false;
	bool escaped = false; // only valid inside string literals
	std::string stringLiteral;  // current string literal
	while (true) {
		if (x == s.end()){
			if (x != a){
				if (mInComment){
					mMultiLineComment = mMultiLineComment + std::string(a,x);
				} else {
					mDelegate->onToken (a, x);
				}
			}
			return true; // also empty line
		}
		Iterator y = x + 1;
		
		// 1. (Multi line) C-Style comments
		if (mInComment){
			if (y != s.end() && *x == '*' && *y == '/'){
				bool v = mDelegate->onComment (mMultiLineComment + std::string (a, x));
				if (!v) return false;
				x = x + 2;
				a = x;
				mInComment = false;
				mMultiLineComment.clear ();
				continue;
			} else {
				x++;
				continue;
			}
		}
		if (y != s.end() && *x == '/' && *y == '*'){
			if (a != x){
				bool v = mDelegate->onToken (a,x);
				if (!v) return false;
			}
			mInComment = true;
			a = y + 1;
			x = y + 1;
			mMultiLineComment.clear();
			continue;
		}

		// 2. C++ Style Comments
		if (y != s.end() && *x == '/' && *y == '/'){
			if (a != x){
				bool v = mDelegate->onToken (a,x);
				if (!v) return false;
			}
			return mDelegate->onLineComment (std::string (y + 1, s.end()));
		}
		
		// 3. string literals
		if (!inStringLiteral && *x == '\"'){
			if (a != x){
				bool v = mDelegate->onToken (a,x);
				if (!v) return false;
			}
			inStringLiteral = true;
			stringLiteral.clear ();
			if (mDelegate->parseStringLiterals()){
				a = x + 1;
				x = x + 1;
			} else {
				a = x;
			}
			continue;
		}
		if (inStringLiteral){
			if (*x == '\"' && !escaped){
				if (mDelegate->parseStringLiterals()){
					bool v = mDelegate->onStringLiteral (stringLiteral);
					if (!v) return false;
				} else {
					bool v = mDelegate->onToken (a, x + 1);
					if (!v) return false;
				}
				inStringLiteral = false;
				escaped = false;
				a = x + 1;
				x = x + 1;
				stringLiteral.clear ();
				continue;
			}
			if (*x == '\\' && !escaped){
				escaped = true;
				x++;
				continue;
			}
			if (*x == '\\' && escaped) {
				escaped = false;
				stringLiteral.push_back ('\\');
				x++;
				continue;
			}
			if (*x == 'n' && escaped){
				escaped = false;
				stringLiteral.push_back ('\n');
				x++;
				continue;
			}
			if (*x == 'b' && escaped){
				escaped = false;
				stringLiteral.push_back ('\b');
				x++;
				continue;
			}
			if (*x == 't' && escaped) {
				escaped = false;
				stringLiteral.push_back ('\t');
				x++;
				continue;
			}
			escaped = false; // no other valid?!
			stringLiteral.push_back (*x);
			x++;
			continue;
		}
		// 3b. Character Literals
		if (y != s.end() && *x == '\''){
			Iterator z = y + 1;
			if (z == s.end()){
				fprintf (stderr, "No end of '\n");
				return false;
			}
			if (*y == '\\'){
				Iterator z1 = z + 1;
				if (z1 == s.end()) {
					fprintf (stderr, "No end of \\\n");
					return false;
				}
				if (*z1 != '\''){
					fprintf (stderr, "Expeceted '\n");
					return false;
				}
				char c = 0;
				if (*z == 'n'){
					c = '\n';
				}
				if (*z == 'b'){
					c = '\b';
				}
				if (*z == 't'){
					c = '\t';
				}
				if (*z == '\''){
					c = '\'';
				}
				if (!c) {
					fprintf (stderr, "Unknown escape sequence \\%c\n", *z);
					return false;
				}
				bool v = false;
				if (mDelegate->parseStringLiterals())
					v = mDelegate->onCharacterLiteral (c);
				else
					v = mDelegate->onToken (x,z1 + 1);
				if (!v) return false;
				a = z1 + 1;
				x = a;
				continue;
			}
			bool v = false;
			if (mDelegate->parseStringLiterals())
				v = mDelegate->onCharacterLiteral (*y);
			else
				v = mDelegate->onToken (x, z + 1);
			if (!v) return false;
			a = z + 1;
			x = a;
			continue;
		}
		// 4. Special double character Tokens
		if (y != s.end() &&
				(  (*x == '<' && *y == '<')  // pipe out
				|| (*x == '>' && *y == '>')  // pipe in
				|| (*x == '+' && *y == '+')  // ++
				|| (*x == '-' && *y == 'y')  // --
				|| (*x == '=' && *y == '=')  // ==
				|| (*x == ':' && *y == ':')  // ::
				)){
			if (a != x){
				bool v = mDelegate->onToken (a,x);
				if (!v) return false;
			}
			bool v = mDelegate->onToken (x, y + 1);
			if (!v) return false;
			a = y + 1;
			x = a;
			continue;
		}
		// 5. Empty Character
		if (isspace(*x)){
			if (a != x) mDelegate->onToken (a,x);
			a = x + 1;
			x = a;
			continue;
		}
		
		// 6. Special Characters
		if (*x == '<' || *x == '>' || *x == '(' || *x == ')' || *x == '{' || *x == '}' || *x == '*' || *x == '&' || *x == ':' || *x == ';' || *x == '+' || *x == '-' || *x == ',' || *x == '[' || *x == ']'){
			if (a != x) mDelegate->onToken (a,x);
			bool v = mDelegate->onSyntactic (*x);
			if (!v) return false;
			a = x + 1;
			x = a;
			continue;
		}
		// 7. Regular Token (just going forward)
		x++;
	}
	if (inStringLiteral){
		fprintf (stderr, "Did not found end of string literal\n");
		return false;
	}
	return true;
}

bool Preprocessor::pushEnd (){
	if (mInComment) {
		fprintf (stderr, "Comment until end of file!\n");
		return false;
	}
	return mDelegate->onEnd();
}

bool Preprocessor::parse (const std::string & content) {
	size_t p = content.find ('\n');
	size_t b = 0;
	while ((p = content.find ('\n', b) != content.npos)){
		std::string line = content.substr (b, p - b);
		bool ret = pushLine (line);
		if (!ret) return false;
		b = p + 1;
	}
	bool ret = pushEnd ();
	return ret;
}

