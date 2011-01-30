#include <sfserialization/autoreflect.h>

enum TestEnum {
	Alpha, Beta, Gamma, Delta
};

// Autorefelct cannot handle this yet:
// enum TestComplexEnum {
//	A = 5 + 3, B = (6 == 6)
// };

// But this
enum TestComplexEnum {
	A = 8, B = 6
};

// And this
enum TestBitEnum {
	Bit0 = 0x1,
	Bit1 = 0x2,
	Bit2 = 0x4,
	Bit3 = 0x8,
	Bit4 = 0x10,
	Bit5 = 0x20,
	Bit6 = 0x40,
	Bit7 = 0x80,
	Bit8 = 0x100,
	Bit9 = 0x200,
	Bit10 = 0x400,
	Bit11 = 0x800,
	Bit12 = 0x1000,
	Bit13 = 0x2000,
	Bit14 = 0x4000,
	Bit15 = 0x8000,
	Bit16 = 0x10000
};

// An empty enum
enum TestEmptyEnum {

};

// Enum inside a class
class Bla {
public:
	enum BlaEnum { A, B, C};
};

SF_AUTOREFLECT_ENUM (TestEnum);
SF_AUTOREFLECT_ENUM (TestComplexEnum);
SF_AUTOREFLECT_ENUM (TestBitEnum);
SF_AUTOREFLECT_ENUM (TestEmptyEnum);
SF_AUTOREFLECT_ENUM (Bla::BlaEnum);
