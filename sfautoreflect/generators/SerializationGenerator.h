#pragma once
#include "CppGeneratorBase.h"

/**
 * A Generator for serialize / deserialize-methods compatible
 * with the sfserialization library.
 */
class SerializationGenerator : public CppGeneratorBase {
public:
	// override
	virtual bool generate (const RootElement * tree);

	virtual const char * name () const { return "SerializationGenerator"; }
	virtual const char * desc () const {
		return "Generates (de)serialization operators for sfserialization library";
	}

protected:

	// override
	virtual bool handleClassUp (const ClassElement * e);

private:
	/// (In class type, as a member function)
	bool generateSerializer (const ClassElement * element);

	/// Generates isDefault() functions
	bool generateIsDefault (const ClassElement * element);

	/// Generate type name function
	bool generateGetCmdName (const ClassElement * element);

	/// Generates deserializer functions (SERIAL)
	/// (In class type, as a member function)
	bool generateDeserializer (const ClassElement * element);
};
