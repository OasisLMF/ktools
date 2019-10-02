add_executable(validateitems)
target_sources(validateitems
	PRIVATE
		validateitems.cpp main.cpp
)

if (WIN32)
	target_link_libraries(validateitems wingetopt)
endif()
