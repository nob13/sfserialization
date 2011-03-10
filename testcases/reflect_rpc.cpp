#include "reflect_rpc.h"
#include "test.h"
#include <sfserialization/Deserialization.h>

int main (int argc, char * argv[]) {
	RpcControlleable controllable;

	// sample command
	SampleCommand commandPackage;
	commandPackage.msg = "Hello World";

	// packing and unpacking
	sf::String cmd;
	sf::Deserialization d (sf::toJSONCmd (commandPackage), cmd);

	// sending to the rpc controllable class
	controllable.handleRpc ("Me", cmd, d);

	// Reached?
	tassert (controllable.received() == "Hello World");

	// Other ways to send command
	bool suc = controllable.handleRpc ("Me", "sampleCommand", std::string("{\"msg\":\"hi\"}"));
	tassert (suc && controllable.received() == "hi");
	suc = controllable.handleRpc ("You", "otherCommand", std::string("{\"msg\":\"dude\"}"));
	tassert (suc && controllable.received() == "dude");

	// error stuff
	suc = controllable.handleRpc ("They", "invalidCommand", std::string ("{}"));
	tassert (!suc, "must recognize invalid commands");
	suc = controllable.handleRpc ("You", "sampleCommand", std::string ("{parsing error}"));
	tassert (!suc, "must recognize invalid serialization");

	return 0;
}
