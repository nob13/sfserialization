#include "SerializationGenerator.h"
#include <stdio.h>

bool SerializationGenerator::generate (const RootElement * tree) {
	StackElement::CommandSet commands = tree->subCommands ();
	if (!(
			commands.count ("SERIAL") ||
			commands.count ("SERIAL_DESERIAL") ||
			commands.count ("SD") ||
			commands.count ("SDC") ||
			commands.count ("GETCMDNAME"))){
		return true; // nothing to do
	}

	fprintf (mOutput,
			"#include <sfserialization/Serialization.h>\n"
			"#include <sfserialization/Deserialization.h>\n");
	return CppGeneratorBase::generate (tree);
}

/*static*/ std::string SerializationGenerator::commandName (const std::string & name){
	std::string result = name;
	result[0] = (char) tolower (name[0]);
	return result;
}


bool SerializationGenerator::handleClassUp (const ClassElement * e) {
	if (!CppGeneratorBase::handleClassUp (e)) return false;
	bool serial   = e->commands.count ("SERIAL") > 0
			|| e->commands.count ("SERIAL_DESERIAL") > 0
			|| e->commands.count ("SD") > 0
			|| e->commands.count ("SDC") > 0;
	bool deserial = e->commands.count ("DESERIAL") > 0
			|| e->commands.count ("SERIAL_DESERIAL")
			|| e->commands.count ("SD") > 0
			|| e->commands.count ("SDC") > 0;

	bool isDefault = serial;
	bool getCmdName = e->commands.count ("GETCMDNAME") > 0 || e->commands.count ("SDC");
	if (serial || deserial) fprintf (mOutput, "\n"); // nicer
	if (serial) {
		bool v = generateSerializer (e); if (!v) return false;
	}
	if (deserial){
		bool v = generateDeserializer (e); if (!v) return false;
	}
	if (isDefault) {
		bool v = generateIsDefault (e); if (!v) return false;
	}
	if (getCmdName) {
		bool v= generateGetCmdName (e); if (!v) return false;
	}
	return true;
}

bool SerializationGenerator::generateSerializer (const ClassElement * element) {
	fprintf (mOutput, "void %sserialize (sf::Serialization& _serialization) const {\n", classScope().c_str());
	for (ClassElement::ParentVec::const_iterator i = element->parents.begin(); i != element->parents.end(); i++){
		if (i->first != Private){
			fprintf (mOutput, "\t%s::serialize(_serialization);\n", i->second.c_str());
		}
	}
	for (ClassElement::MemberVariableVec::const_iterator i = element->memberVariables.begin(); i != element->memberVariables.end(); i++){
		if (i->first != Private)
			fprintf (mOutput, "\t_serialization (\"%s\", %s);\n", i->second.name.c_str(), i->second.name.c_str());
	}
	fprintf (mOutput, "}\n\n");
	return true;
}

bool SerializationGenerator::generateIsDefault (const ClassElement * element) {
	fprintf (mOutput, "bool %sisDefault () const {\n", classScope().c_str());
	fprintf (mOutput, "\treturn true\n");
	for (ClassElement::ParentVec::const_iterator i = element->parents.begin(); i != element->parents.end(); i++){
		if (i->first != Private){
			fprintf (mOutput, "\t\t&& %s::isDefault()\n", i->second.c_str());
		}
	}
	for (ClassElement::MemberVariableVec::const_iterator i = element->memberVariables.begin(); i != element->memberVariables.end(); i++){
		if (i->first != Private){
			fprintf (mOutput, "\t\t&& sf::isDefault(%s)\n", i->second.name.c_str());
		}
	}
	fprintf (mOutput, "\t;\n");
	fprintf (mOutput, "}\n\n");
	return true;
}

bool SerializationGenerator::generateGetCmdName (const ClassElement * element) {
	fprintf (mOutput, "const char* %sgetCmdName () {\n", classScope().c_str());
	fprintf (mOutput, "\treturn \"%s\";\n", commandName (element->name).c_str());
	fprintf (mOutput, "}\n\n");
	return true;
}


bool SerializationGenerator::generateDeserializer (const ClassElement * element) {
	fprintf (mOutput, "bool %sdeserialize (const sf::Deserialization& _deserialization){\n", classScope().c_str());
	fprintf (mOutput, "\tbool suc = true;\n");
	for (ClassElement::ParentVec::const_iterator i = element->parents.begin(); i != element->parents.end(); i++){
		if (i->first != Private){
			fprintf (mOutput, "\tsuc=%s::deserialize(_deserialization) && suc;\n", i->second.c_str());
		}
	}
	for (ClassElement::MemberVariableVec::const_iterator i = element->memberVariables.begin(); i != element->memberVariables.end(); i++){
		if (i->first != Private)
			fprintf (mOutput, "\tsuc = _deserialization (\"%s\", %s) && suc;\n", i->second.name.c_str(), i->second.name.c_str());
	}
	fprintf (mOutput, "\treturn suc;\n");
	fprintf (mOutput, "}\n\n");
	return true;
}

