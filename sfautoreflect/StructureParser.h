#pragma once
#include "Preprocessor.h"
#include <sstream>
#include <string>
#include <stack>
#include <vector>
#include <set>

#include "StructureTypes.h"

/**
 * A PreprocessorDelegate which parses the line and tries to extract the basic file structure.
 */
class StructureParser : public PreprocessorDelegate {
public:
	StructureParser ();
	~StructureParser ();

	// Clears current state
	void clear ();

	// Prints out mDebug info
	void setDebug (bool v = true) { mDebug = v; }

	// Implementation of PreprocessorDelegate
	virtual bool onToken (Iterator begin, Iterator end);
	virtual bool onSyntactic (char c);
	virtual bool onEnd ();
	virtual bool parseStringLiterals () const { return false; } // we want them as plain token

	typedef std::vector<std::string> StringVec;

	/// Gives you access to the root element (after parsing)
	const RootElement * root () const;

	/// Sets an own prefix for commands (default: AUTOREFLECT_)
	void setPrefix (const std::string & prefix) { mCommandPrefix = prefix; }
private:
	typedef std::stack<StackElement*> ParseStack;

	ParseStack mStack;				///< Current stack of structure
	StringVec  mIncomingLine;		///< Flowing in elements on a line (ending with ;) not matched yet
	bool       mDebug;				///< Print mDebug info
	RootElement * mRoot;
	std::string mCommandPrefix;		///< Prefix for all commands

	/// Tries to match an incoming line
	/// finit ... the finishing element
	bool matchLine (char finishing);

	/// Matches a type name in a string vector
	/// Used for base types in struct / class definitions (no const handling!)
	/// Returns success state in result
	static StringVec::const_iterator matchTypeName (StringVec::const_iterator begin, StringVec::const_iterator end, bool * result);

	/// Formats a type (which was found by matchTypeName)
	static std::string formatType (StringVec::const_iterator begin, StringVec::const_iterator end);

	/// Tries to match a C++ type (including static etc.)
	static StringVec::const_iterator matchCppType (StringVec::const_iterator begin, StringVec::const_iterator end, bool * result, CppType * type);

	/// Checks whether a string is a regular identifier
	static bool isRegularIdentifier (const std::string & id);

	/// Tries to match a variable definition and saves it in definition
	/// Variable definition has to start from begin to end (no premature end)
	/// Returns true on success
	static bool matchVariableDefinition (StringVec::const_iterator begin, StringVec::const_iterator end, VariableDefinition * definition);

	/// Tries to match a Argument definition
	static StringVec::const_iterator matchArgumentDefinition (StringVec::const_iterator begin, StringVec::const_iterator end, bool * result, ArgumentDefinition * definition);

	/// Tries to match a function head
	static bool matchFunctionDeclaration (StringVec::const_iterator begin, StringVec::const_iterator end, FunctionDeclarationElement * declaration);

	/// Pushing a structure element
	void push (StackElement * elem);

	/// Pops a structure element (does NOT delete it!, still alive in children section of StackElement)
	bool pop ();

public:
	// self Testing functions
	static bool test_matchTypeName ();
	static bool test_matchCppType ();
	static bool test_matchVariableDefinition ();
	static bool test_matchFunctionDeclaration ();
};
