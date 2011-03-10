#include "EnumGenerator.h"
#include "StaticHashTableBuilder.h"
#include <stdio.h>

bool EnumGenerator::generate (const RootElement * tree)  {
	StackElement::CommandSet commands = tree->subCommands ();
	if (!(
			commands.count ("ENUM_TOSTRING") ||
			commands.count ("ENUM_TOFROMSTRING") ||
			commands.count ("ENUM"))){
		return true; // nothing to do
	}

	fprintf (mOutput, "#include <assert.h>\n");
	fprintf (mOutput, "#include <string.h>\n");
	return CppGeneratorBase::generate (tree);
}

bool EnumGenerator::handleEnumUp (const EnumElement * e) {
	if (!CppGeneratorBase::handleEnumUp (e)) return false;
	bool toString   = e->commands.count ("ENUM_TOSTRING") > 0
			|| e->commands.count ("ENUM_TOFROMSTRING") > 0
			|| e->commands.count ("ENUM") > 0;
	bool fromString = e->commands.count ("ENUM_FROMSTRING") > 0
			|| e->commands.count ("ENUM_TOFROMSTRING") > 0
			|| e->commands.count ("ENUM") > 0;

	if (toString){
		bool v = generateToString (e); if (!v) return false;
	}
	if (fromString){
		bool v = generateFromString (e); if (!v) return false;
	}
	return true;
}

/// Checks if an enum is complex (has more than one trivial initializer (element[0]->values[0]=='0'))
static bool isComplex (const EnumElement * element){
	if (element->values.empty()) return false;
	if (!element->values[0].second.empty() && element->values[0].second != "0") return true;
	for (EnumElement::ValueVec::const_iterator i = element->values.begin() + 1; i != element->values.end(); i++){
		if (!i->second.empty()) return true;
	}
	return false;
}

bool EnumGenerator::generateToString (const EnumElement * element) {
	bool complex = isComplex (element);
	fprintf (mOutput, "const char* toString (%s%s e){\n", classScope().c_str(), element->name.c_str());
	if (element->values.empty()){
		fprintf (mOutput, "\tassert (false && \"Enum has no values\");\n");
		fprintf (mOutput, "\treturn \"invalid\";\n");
		fprintf (mOutput, "}\n\n");
		return true;
	}
	if (complex) {
		// no string table O(n)
		fprintf (mOutput, "\t// Note: not generating string table because of non-trivial enum initializers\n");
		for (EnumElement::ValueVec::const_iterator i = element->values.begin(); i != element->values.end(); i++){
			fprintf (mOutput, "\tif (e == %s) return \"%s\";\n", i->first.c_str(), i->first.c_str());
		}
		fprintf (mOutput, "\tassert (false && \"undefined enum value\");\n");
		fprintf (mOutput, "\treturn \"UNDEFINED\";\n");
	} else {
		// with string table
		fprintf (mOutput, "\tconst char* values[] = {\n");
		bool first = true;
		for (EnumElement::ValueVec::const_iterator i = element->values.begin(); i != element->values.end(); i++){
			fprintf (mOutput, "\t\t%s \"%s\"\n", first ? "" : ",", i->first.c_str());
			first = false;
		}
		fprintf (mOutput, "\t};\n");

		fprintf (mOutput, "\tint x = (int) e;\n");
		fprintf (mOutput, "\tif (x < 0 || x >= %ld){\n", element->values.size());
		fprintf (mOutput, "\t\tassert (false && \"undefined enum value\");\n");
		fprintf (mOutput, "\t\treturn \"UNDEFINED\";\n");
		fprintf (mOutput, "\t}\n");
		fprintf (mOutput, "\treturn values[x];\n");
	}
	fprintf (mOutput, "}\n\n");
	return true;
}

bool EnumGenerator::generateFromString (const EnumElement * element) {
	std::string enumName = classScope () + element->name;

	fprintf (mOutput, "bool fromString (const char* key, %s &e){\n", enumName.c_str());
	if (!element->values.empty()){

		// Generating Hash Table
		StaticHashTableBuilder generator;
		for (EnumElement::ValueVec::const_iterator i = element->values.begin(); i != element->values.end(); i++){
			generator.add(i->first, classScope() + i->first);
		}
		typedef StaticHashTableBuilder::HashTable HashTable;
		bool suc = generator.generateHashCode(mOutput, enumName.c_str());
		if (!suc){
			fprintf (stderr, "StaticHashTableBuilder failed\n");
			return false;
		}
		fprintf (mOutput, "\tif (foundKey) { e = value; return true; }\n");
		fprintf (mOutput, "\t// did not found key\n");
	}
	fprintf (mOutput, "\treturn false;\n");
	fprintf (mOutput, "}\n\n");
	return true;
}

