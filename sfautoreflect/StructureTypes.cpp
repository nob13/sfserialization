#include "StructureTypes.h"

const char * toString (Visibility v) {
	const char * values [] = { "Public", "Private", "Protected" };
	const int l = sizeof (values) / sizeof (const char*);
	int x = (int) v;
	if (x < 0 || x >= l) return "invalid";
	return values[x];
}

const char * toString (StackElement::Type v){
	const char * values[] =  { "Root", "Namespace", "Class", "Function", "Enum", "Code", "Unknown" };
	const int l = sizeof (values) / sizeof (const char*);
	int x =(int) v;
	if (x < 0 || x >= l) return "invalid";
	return values[x];

}
