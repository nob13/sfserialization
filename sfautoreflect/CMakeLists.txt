# Main Library
set (reflect_files 
	"Preprocessor.cpp" 
	"Preprocessor.h" 
	"StructureTypes.h" 
	"StructureTypes.cpp" 
	"StructureParser.h" 
	"StructureParser.cpp"
	"Configuration.h"
	"Configuration.cpp" 
	"generators/Generator.h" 
	"generators/Generator.cpp"
	"generators/CppGeneratorBase.h"
	"generators/CppGeneratorBase.cpp"
	"generators/DumpGenerator.h"
	"generators/DumpGenerator.cpp"
	"generators/StaticHashTableBuilder.h"
	"generators/StaticHashTableBuilder.cpp"
	"generators/EnumGenerator.h"
	"generators/EnumGenerator.cpp"
	"generators/SerializationGenerator.h"
	"generators/SerializationGenerator.cpp"
	"generators/RpcGenerator.h"
	"generators/RpcGenerator.cpp"
)

add_library (reflect ${reflect_files})

# Main Executable
set (files "main.cpp")
add_executable (sfautoreflect ${files})
target_link_libraries (sfautoreflect reflect ${LIBS})

install (TARGETS  sfautoreflect RUNTIME DESTINATION bin)
