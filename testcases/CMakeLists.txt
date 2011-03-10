macro (add_automatic_test name)
	message (STATUS "Adding test ${name}")
	add_executable ("testcase_${name}" "${name}.cpp")
	target_link_libraries ("testcase_${name}" ${LIBS})
	add_test (${name} "testcase_${name}")
endmacro()

add_automatic_test (basics)
add_automatic_test (serialization_regression)
add_automatic_test (serialization)
add_automatic_test (json)
add_automatic_test (sample)

#
# Start autoreflect on all files in src_files and put created
# files into the list gen_files.
#
macro (autoreflect_decorate gen_files src_files)
	set (listcopy ${ARGV})
	list (REMOVE_AT listcopy 0) # Removing two parameters
	foreach (src ${listcopy})
		set (out "${src}_reflect.cpp")
		
		set (src_path "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
		set (out_path "${CMAKE_CURRENT_BINARY_DIR}/${out}")

		# autoreflect won't create directory by itself
		get_filename_component (out_path_dir ${out_path} PATH)
		file (MAKE_DIRECTORY ${out_path_dir})
		
		add_custom_command (OUTPUT ${out} DEPENDS ${src}
		COMMAND sfautoreflect ${src_path} -o ${out_path})
		list (APPEND ${gen_files} ${out})
	endforeach ()
endmacro(autoreflect_decorate)

# Scans all files in ${files} for the SF_AUTOREFLECT_ macro
# and automatically run autoreflect on them. Also appends the
# list with the created files.
macro (autoreflect_decorate_auto files)
	set (original ${${files}})
	foreach (f ${original})
		file (STRINGS ${f} found LIMIT_COUNT 1 REGEX SF_AUTOREFLECT_)
		if (found)
			message (STATUS "   - ${f} contains (problably) reflection code")
			autoreflect_decorate (${files} ${f})
		endif() 
	endforeach ()
endmacro ()


# Add a (reflected) testcase.
macro (autoreflect_testcase name)
	set (src_files ${name}.h ${name}.cpp)
	autoreflect_decorate_auto (src_files)
	add_executable (testcase_${name} ${src_files})
	target_link_libraries ("testcase_${name}" ${LIBS})
	add_test (testcase_${name} testcase_${name})
endmacro ()

autoreflect_testcase (reflect_enum)
autoreflect_testcase (reflect_struct)
autoreflect_testcase (reflect_rpc)
autoreflect_testcase (performance)
