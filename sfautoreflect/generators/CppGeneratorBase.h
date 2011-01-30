#pragma once
#include "Generator.h"

/**
 * A common base class for Generator implementations which
 * already provides Structure-Tree traversation and namespace
 * generation. Its meant for Generators generating Cpp-Source.
 *
 * You can hook into the generate and the handle-methods.
 */
class CppGeneratorBase : public Generator {
public:
	CppGeneratorBase ();
	~CppGeneratorBase ();

	// Implementation of Generator
	virtual void setOutput (FILE * output);
	// virtual bool setOutputFile (const char * filename);
	virtual bool generate (const RootElement * tree);

protected:
	/// Recursive generating function
	virtual bool recGenerate (const StackElement * element);

	/// a namespace is put on the stack
	virtual bool handleNamespaceUp (const NamespaceElement * e);

	/// a namespace is took from the stack
	virtual bool handleNamespaceDown (const NamespaceElement * e);

	/// a class is took on the stack
	virtual bool handleClassUp (const ClassElement * e);

	/// a class is took from the stack
	virtual bool handleClassDown (const ClassElement * e);

	/// a enum is took on the stack
	virtual bool handleEnumUp (const EnumElement * e);

	/// a enum is took from the stack
	virtual bool handleEnumDown (const EnumElement * e);

	/// Returns the current class scope (including :: at the end, if there is an class scope)
	std::string classScope ();

	FILE * mOutput;
	bool mOwnFile;///< If we openend the file
	std::vector<std::string> mClassScope;
};
