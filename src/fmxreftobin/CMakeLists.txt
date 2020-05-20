add_executable(fmxreftobin)
target_sources(fmxreftobin
	PRIVATE
		fmxreftobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmxreftobin wingetopt)
endif()

install(TARGETS fmxreftobin RUNTIME DESTINATION bin)