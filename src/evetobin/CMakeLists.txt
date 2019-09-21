add_executable(evetobin)
target_sources(evetobin
	PRIVATE
		evetobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(evetobin wingetopt)
endif()