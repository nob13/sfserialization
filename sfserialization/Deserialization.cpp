#include "Deserialization.h"

namespace sf {

Deserialization::Deserialization () {
}

Deserialization::Deserialization (const std::string & s){
	mText = s;
	/// With command compatibility
	std::string cmd;
	mObject.init (mText.c_str(), cmd);
}

Deserialization::Deserialization (const std::string & s, std::string & cmd){
	mText = s;
	mObject.init (mText.c_str(), cmd);
}

Deserialization::Deserialization (const ByteArrayBase & array) {
	mObject.init (&array.front(), array.size());
}

Deserialization::Deserialization (const ByteArrayBase & array, std::string & cmd) {
	mObject.init (&array.front(), cmd, array.size());
}

Deserialization::Deserialization (const json::Object & o){
	mObject = o;
}

void Deserialization::streamOut (std::ostream & stream) const {
	stream << mObject;
}

}

std::ostream & operator<< (std::ostream & stream, const sf::Deserialization & ds) {
	ds.streamOut (stream);
	return stream;
}
