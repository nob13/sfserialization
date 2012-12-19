#include "Configuration.h"
#include <stdio.h>

#include "generators/EnumGenerator.h"
#include "generators/DumpGenerator.h"
#include "generators/SerializationGenerator.h"
#include "generators/RpcGenerator.h"

Configuration::Configuration () {
	mPredebug = false;
	mHelp = false;
	mDebug = false;
	mVersion = false;

	Generator * dumpGenerator = new DumpGenerator;
	Generator * enumGenerator = new EnumGenerator;
	Generator * serializationGenerator = new SerializationGenerator;
	Generator * rpcGenerator = new RpcGenerator;
	mGenerators["dump"] = dumpGenerator;
	mGenerators["enum"] = enumGenerator;
	mGenerators["sd"]   = serializationGenerator;
	mGenerators["rpc"]  = rpcGenerator;
	mDefaultGenerators.push_back (enumGenerator);
	mDefaultGenerators.push_back (serializationGenerator);
	mDefaultGenerators.push_back (rpcGenerator);
}

Configuration::~Configuration () {
	for (GeneratorMap::iterator i = mGenerators.begin(); i != mGenerators.end(); i++){
		delete i->second;
	}
}


/// Parse command line arguments.
bool Configuration::parse (int argc, char * argv[]){
	bool waitForOutputFile = false;
	bool waitForGenerator  = false;
	for (int i = 1; i < argc; i++){
		std::string arg = argv[i];
		if (waitForOutputFile) {
			if (!mOutputFilename.empty()){
				fprintf (stderr, "Already specified an output filename\n");
				return false;
			}
			mOutputFilename = arg;
			waitForOutputFile = false;
			continue;
		}
		if (waitForGenerator) {
			std::string generatorName = arg;
			GeneratorMap::iterator i = mGenerators.find (generatorName);
			if (i == mGenerators.end()){
				fprintf (stderr, "Unknown Generator: %s\n", generatorName.c_str());
				return false;
			}
			mSelectedGenerators.push_back (i->second);
			waitForGenerator = false;
			continue;
		}
		if (arg == "--predebug") { mPredebug = true; continue; }
		if (arg == "--debug")    { mDebug = true; continue; }
		if (arg == "-o")       { waitForOutputFile = true; continue; }
		if (arg == "-g")       { waitForGenerator  = true; continue; }
		if (arg == "--help")   { mHelp = true; continue; }
		if (arg == "-v")       { mVersion = true; continue; }

		if (!mFilename.empty()){
			fprintf (stderr, "Can only specify one filename\n");
			return false;
		}
		mFilename = argv[i];
	}
	if (!hasFilename()) return false;

	if (mSelectedGenerators.empty()) {
		mSelectedGenerators = mDefaultGenerators;
	}

	return true;
}

/// Prints out mHelp info
void Configuration::printInfo () {
	printf ("sfautoreflect 0.2.5\n");
	if (mVersion){
		return;
	}
	fprintf (stderr, "Copyright 2011-2012 Norbert Schultz\n");
	fprintf (stderr, "Usage: sfautoreflect HEADER_FILE [-o OUTPUT_FILE] [-g Generator] [OPTIONS]\n");
	fprintf (stderr, "Automated tool for generating C++ meta information\n");
	fprintf (stderr, " -v Prints version number\n");
	fprintf (stderr, " --predebug Prints debug info for preprocessor / tokenizer (No generation)\n");
	fprintf (stderr, " --debug    Prints debug info of the parser\n");
	fprintf (stderr, " --help     Prints this help\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "Generator Selection:\n");
	for (GeneratorMap::const_iterator i = mGenerators.begin(); i != mGenerators.end(); i++) {
		Generator * g (i->second);
		fprintf (stderr, "-g %s - %s\n", i->first.c_str(), g->name());
		fprintf (stderr, "\t%s\n", g->desc());
	}
	fprintf (stderr, "Default Generators: ");
	for (GeneratorVec::const_iterator i = mDefaultGenerators.begin(); i != mDefaultGenerators.end(); i++) {
		fprintf (stderr, "%s ", (*i)->name());
	}
	fprintf (stderr, "\n");
}

void Configuration::printErr  () {
	printInfo ();
}

