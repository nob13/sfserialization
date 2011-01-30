#pragma once
#include "../StructureTypes.h"
#include <string>
#include <vector>

/**
 * Base class for all generators. Generate some text into a C-Stream out of a
 * parsed header file.
 *
 */
class Generator {
public:
	/// 1. Creating Generator
	Generator ();
	virtual ~Generator();
	
	/// 2. Set output stream (default: stdout)
	virtual void setOutput (FILE * output) = 0;
	
	/// 3. Generate data
	/// Returns true on success
	virtual bool generate (const RootElement * tree) = 0;

	/// Returns name of the generator
	virtual const char * name () const = 0;
	/// Returns description of the generator
	virtual const char * desc () const = 0;

};
