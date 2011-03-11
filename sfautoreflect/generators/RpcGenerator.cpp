#include "RpcGenerator.h"
#include "StaticHashTableBuilder.h"
#include "SerializationGenerator.h"

bool RpcGenerator::generate (const RootElement * tree) {
	StackElement::CommandSet commands = tree->subCommands ();
	if (!(commands.count ("RPC"))){
		return true; // nothing to do
	}

	fprintf (mOutput,
			"#include <sfserialization/Serialization.h>\n"
			"#include <sfserialization/Deserialization.h>\n");
	return CppGeneratorBase::generate (tree);
}


bool RpcGenerator::handleClassUp (const ClassElement * e) {
	if (!CppGeneratorBase::handleClassUp(e)) return false;
	if (!e->commands.count ("RPC")) return true; // nothing to do

	typedef std::vector<std::string> RpcFuncVector;
	RpcFuncVector rpcFuncs;
	// Scanning for onRpc methods.
	for (ClassElement::ChildrenVec::const_iterator i = e->children.begin(); i != e->children.end(); i++) {
		StackElement * element (*i);
		if (element->type == StackElement::FunctionDeclaration){
			if (element->name.substr (0, 5) == "onRpc") {
				FunctionDeclarationElement * func = static_cast<FunctionDeclarationElement*> (element);
				std::string type;
				if (!checkRpcFuncArguments (func, &type)) continue;
				rpcFuncs.push_back (type);
			}
		}
	}
	if (rpcFuncs.empty()){
		fprintf (stderr, "Did not found any RPC functions in %s\n", sf::toJSON (*e).c_str());
		return false;
	}

	// Generate name ()
	fprintf (mOutput, "const char * %sname() const {\n", classScope().c_str());
	fprintf (mOutput, "\treturn \"%s\";\n", e->name.c_str());
	fprintf (mOutput, "}\n\n");

	// Generate commands ()
	fprintf (mOutput, "const char * g%s_commands[] = {\n", e->name.c_str());
	for (RpcFuncVector::const_iterator i = rpcFuncs.begin(); i != rpcFuncs.end(); i++) {
		fprintf (mOutput, "\t\"%s\",\n", SerializationGenerator::commandName (*i).c_str());
	}
	fprintf (mOutput, "\t0\n};\n\n");
	fprintf (mOutput, "const char ** %scommands () const {\n", classScope().c_str());
	fprintf (mOutput, "\treturn g%s_commands;\n", e->name.c_str());
	fprintf (mOutput, "}\n\n");

	// Call function, necessary to call the final member function via a function pointer
	// As a template it can also circumvent visibility problems.
	fprintf (mOutput, "template <class C, class P, void (C::*handler) (const sf::HostId &, const P &, const sf::ByteArray & )>\n"
			        	"\tstatic bool callRpcHandler (C * instance, const sf::HostId & sender, const sf::Deserialization & header, const sf::ByteArray & data){\n"
						"\tP p;\n"
						"\tbool suc = p.deserialize (header);\n"
						"\tif (!suc) return false;\n"
						"\t(instance->*handler) (sender, p, data);\n"
						"\treturn true;\n"
						"}\n\n");

	// Generate handleRpc () method with hash table search
	// Values are function pointers to callRpcHandler instantiations
	StaticHashTableBuilder builder;
	for (RpcFuncVector::const_iterator i = rpcFuncs.begin(); i != rpcFuncs.end(); i++) {
		char buffer [1024];
		buffer [1023] = 0;
		snprintf (buffer, 1023, "&callRpcHandler<%s,%s,&%s::onRpc>", e->name.c_str(), i->c_str(), e->name.c_str());
		builder.add (SerializationGenerator::commandName (*i), buffer);
	}


	fprintf (mOutput, "bool %shandleRpc (const sf::HostId & sender, const sf::String & cmd, const sf::Deserialization & header, const sf::ByteArray & data) {\n", classScope().c_str());
	fprintf (mOutput, "\tif (header.error()) return false;\n");
	fprintf (mOutput, "\tconst char * key = cmd.c_str();\n");
	fprintf (mOutput, "\ttypedef bool (*ValueType) (%s *, const sf::HostId &, const sf::Deserialization &, const sf::ByteArray &);\n", e->name.c_str());
	builder.generateHashCode (mOutput, "ValueType");
	fprintf (mOutput, "\tif (!foundKey) return false;\n");
	fprintf (mOutput, "\tvalue (this, sender, header, data);\n\n");
	fprintf (mOutput, "\treturn true;\n");
	fprintf (mOutput, "}\n");
	return true;
}

bool RpcGenerator::checkRpcFuncArguments (const FunctionDeclarationElement * func, std::string * typeName) {
	if (func->arguments.size() != 3) {
		fprintf (stderr, "Warning: did found a rpc like function with wrong argument count: %s\n", sf::toJSON (func).c_str());
		return false;
	}
	if (func->returnType.name != "void") {
		fprintf (stderr, "Warning: Return type of rpc like function will be ignored: %s!", sf::toJSON (func).c_str());
		// can stil continue
	}
	if (!(/*func->arguments[0].type.name == "HostId &" &&*/ func->arguments[0].type.const_)){
		fprintf (stderr, "Error: 1st argument %s doesn't fit: %s\n", sf::toJSON (func).c_str(), sf::toJSON (func->arguments[0]).c_str());
		return false;
	}
	if (!(func->arguments[1].type.const_)){
		fprintf (stderr, "Error: 2nd argument of %s doesn't fit: %s\n", sf::toJSON (func).c_str(), sf::toJSON (func->arguments[1]).c_str());
		return false;
	}
	if (!(/*func->arguments[1].type.name == "ByteArray &" &&*/ func->arguments[2].type.const_ )){
		fprintf (stderr, "Error: 3rd argument of %s doesn't fit: %s\n", sf::toJSON(func).c_str(), sf::toJSON (func->arguments[2]).c_str());
		return false;
	}
	std::string type = func->arguments[1].type.name; // must be "T &" (const is already checked)
	if (type.empty() || *type.rbegin() != '&') {
		fprintf (stderr, "Error: Type of 2nd argument doesn't fit: const %s\n", type.c_str());
		return false;
	}
	*typeName = type.substr (0, type.length() - 2);
	if (typeName->find(" ") != typeName->npos || typeName->find (" ") != typeName->npos){
		fprintf (stderr, "Warning: Type of 2nd argument looks strange: %s\n", typeName->c_str());
		return false;
	}
	return true;
}

