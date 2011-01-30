#pragma once
#include "CppGeneratorBase.h"

/**
 * A generator for Enum toString, fromString methods.
 */
class EnumGenerator : public CppGeneratorBase {
public:
	// override
	virtual bool generate (const RootElement * tree);

	virtual const char * name () const { return "EnumGenerator"; }
	virtual const char * desc () const { return "Generates toString()/fromString() methods for Enums"; }


protected:
	// override
	virtual bool handleEnumUp (const EnumElement * e);

private:

	/// Generates a toString method for enums
	bool generateToString (const EnumElement * element);

	/// Generates a fromString method for enums
	bool generateFromString (const EnumElement * element);
};
