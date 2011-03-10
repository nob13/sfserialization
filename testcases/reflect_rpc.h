#pragma once
#include <sfserialization/autoreflect.h>
#include <sfserialization/Serialization.h>

namespace sf {
// Some types needed for compilation
typedef ByteArrayBase ByteArray;
typedef std::string String;
typedef std::string HostId;
}

struct SampleCommand {
	sf::String msg;
	SF_AUTOREFLECT_SDC;
};

struct OtherCommand {
	sf::String msg;
	SF_AUTOREFLECT_SDC;
};

class RpcControlleable {
public:
	SF_AUTOREFLECT_RPC;

	/// what was the last package the controllable class received
	const std::string & received () const { return mReceived; }
private:
	// rpc handlers
	void onRpc (const sf::HostId & sender, const SampleCommand & cmd, const sf::ByteArray & data) {
		mReceived = cmd.msg;
	}
	void onRpc (const sf::HostId & sender, const OtherCommand & cmd, const sf::ByteArray & data) {
		mReceived = cmd.msg;
	}

	std::string mReceived;
};
