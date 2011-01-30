#include <stdio.h>
#include <string.h>
#include <string>
#include "Preprocessor.h"
#include "StructureParser.h"

#include "Configuration.h"

/**
 * @file main method of the sfautoreflect tool.
 */

int main (int argc, char * argv[]){
	Configuration configuration;
	if (!configuration.parse(argc, argv)){
		configuration.printInfo();
		return 1;
	}
	
	// Building up parser
	PreprocessorPrintingDelegate printer;
	StructureParser parser;
	if (configuration.debug()) parser.setDebug ();
	
	
	// Connecting to the output delegate (either printer or parser)
	PreprocessorDelegate * delegate;
	if (configuration.predebug()){
		delegate = &printer;
	} else {
		delegate = &parser;
	}
	
	// Loading source file
	Preprocessor preprocessor (delegate); // use printing delegate for debugging preprocessor
	if (!preprocessor.loadFile(configuration.filename().c_str())){
		return 1;
	}

	if (configuration.predebug()){
		return 0;
	}

	// Output
	FILE * outputFile = 0;
	if (configuration.hasOutputFilename()){
		outputFile = fopen (configuration.outputFilename().c_str(), "w");
		if (!outputFile) {
			fprintf (stderr, "Could not write open %s\n", configuration.outputFilename().c_str());
			return 1;
		}
	} else {
		outputFile = stdout;
	}

	/// Output file includes source file
	fprintf (outputFile, "// *****************************************************************************\n");
	fprintf (outputFile, "// This file is auto generated using sfautoreflect (http://cgvis.de/autoreflect)\n");
	fprintf (outputFile, "// All changes will probably overwritten\n");
	fprintf (outputFile, "// Resistance is futile\n");
	fprintf (outputFile, "// *****************************************************************************\n");
	fprintf (outputFile, "#include \"%s\"\n", configuration.filename().c_str());

	for (Configuration::GeneratorVec::const_iterator i = configuration.selectedGenerators().begin(); i != configuration.selectedGenerators().end(); i++) {
		(*i)->setOutput(outputFile);
		(*i)->generate (parser.root());
	}

	if (configuration.hasOutputFilename()){
		fclose (outputFile);
	}
	return 0;
}

