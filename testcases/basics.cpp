#include "StructureParser.h"
#include "Preprocessor.h"
#include "test.h"

int main (int argc, char * argv[]){
	printf ("Basics Testcase\n");
	RUN (StructureParser::test_matchTypeName());
	RUN (StructureParser::test_matchCppType());
	RUN (StructureParser::test_matchVariableDefinition());
	RUN (StructureParser::test_matchFunctionDeclaration());
	return 0;
}
