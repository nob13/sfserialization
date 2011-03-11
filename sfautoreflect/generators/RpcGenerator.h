#pragma once
#include "CppGeneratorBase.h"

/// A generator for RPC-capable objects
/// like used in project Schneeflocke.
/// Used command: SF_AUTOREFLECT_RPC
/// Will generate
/// - const char ** commands () returns all accepted commands
/// - const char * name() will return class name
///
///   // handles an RPC Command; Returns true on success (accepted command, correct parsing)
/// - bool handleRpc (const HostId & sender, const String & cmdName, const Deserialization & header, const ByteArray & data)
/// All methods who want to be reachable do have to have the following signature
/// void onRpc (const HostId & source, const T & cmd, const ByteArray & data);
/// where T is an structure which can be used as a a command (implements serialize/deserialize/cmdName)
/// the cmdName of T must be like SerializationGenerator::commandName of T. (E.g. RequestReply --> Command name is requestReply).
///
/// Warning: this class is highly special for Schneeflocke and subject of further changes.
class RpcGenerator : public CppGeneratorBase {
public:
	// override
	virtual bool generate (const RootElement * tree);

	virtual const char * name () const { return "RPCGenerator"; }
	virtual const char * desc () const { return
			"Generates helper function for JSON-RPC capable objects like being used in schneeflocke library.";
	}

protected:
	// override
	virtual bool handleClassUp (const ClassElement * e);

	// check if rpc function fits, typeName is type name of 2nd argument
	bool checkRpcFuncArguments (const FunctionDeclarationElement * func, std::string * typeName);
};
