#include "DumpGenerator.h"
#include <sfserialization/Serialization.h>
#include <stdio.h>

void DumpGenerator::setOutput (FILE * output) {
	mOutput = output;
}

bool DumpGenerator::generate (const RootElement * tree) {
	fprintf (mOutput, "%s\n", sf::toJSONEx (*tree, sf::INDENT).c_str());
	return true;
}
