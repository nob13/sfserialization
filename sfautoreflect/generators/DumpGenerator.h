#pragma once
#include "Generator.h"

/// A generator which just prints out the parsed tree
/// in JSON
class DumpGenerator : public Generator {
public:
	virtual void setOutput (FILE * output);
	virtual bool generate (const RootElement * tree);


	virtual const char * name () const { return "DumpGenerator"; }
	virtual const char * desc () const { return "Dumps out parsed tree in JSON"; }

private:
	FILE * mOutput;

};
