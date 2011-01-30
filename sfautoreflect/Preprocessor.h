#pragma once
#include <string>

struct PreprocessorDelegate {
	typedef std::string::iterator Iterator;
	virtual ~PreprocessorDelegate (){}

	/// A preproccor instruction
	virtual bool onPreprocess (const std::string & line) { return true; }
	/// A simple token (not something else)
	virtual bool onToken (Iterator begin, Iterator end) { return true; }
	/// A line comment
	virtual bool onLineComment (const std::string & content) { return true; }
	/// A regular comment
	virtual bool onComment (const std::string& content) { return true; }
	/// A string literal
	virtual bool onStringLiteral (const std::string & content) { return true; }
	/// A character literal
	virtual bool onCharacterLiteral (char c) { return true; }
	/// A one character token (like braces etc.)
	virtual bool onSyntactic (char c) { return true; }
	/// End of parsing
	virtual bool onEnd () { return true; }

	/// Tells the Preprocessor to decode string literals (otherwise you will receive them as regular tokens)
	/// Also used on character literals
	virtual bool parseStringLiterals () const { return true; }
};

/// A simple delegate which just prints out each element
struct PreprocessorPrintingDelegate : public PreprocessorDelegate {
	// Implementation of PreprocessorDelegate
	virtual bool onPreprocess (const std::string & line);
	virtual bool onToken (Iterator begin, Iterator end);
	virtual bool onLineComment (const std::string & content);
	virtual bool onComment (const std::string& content);
	virtual bool onStringLiteral (const std::string & content);
	virtual bool onCharacterLiteral (char c);
	virtual bool onSyntactic (char c);
	virtual bool onEnd ();
};

/**
Scans for elements in a C++ Code stream and sorts them into various types
Kicks out Preprocessor functions.
*/
class Preprocessor {
public:
	Preprocessor (PreprocessorDelegate * delegate);
	~Preprocessor ();
	
	/// Loads a file (convenience function for pushLine/pushEnd)
	bool loadFile (const char * filename);
	
	/// Push a new line of source code
	bool pushLine (const std::string & line);
	
	/// Push end of file
	bool pushEnd ();

	/// Parse a whole content
	bool parse (const std::string & content);

	int lineNum () const { return mLine; }
	
	typedef std::string::iterator Iterator;
private:

	PreprocessorDelegate * mDelegate;
	int         mLine;
	bool        mInComment;		///< Currently inside a multi line comment
	std::string mMultiLineComment;	///< Current multi line comment
};

