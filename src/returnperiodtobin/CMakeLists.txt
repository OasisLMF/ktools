add_executable(returnperiodtobin)
target_sources(returnperiodtobin
	PRIVATE
		returnperiodtobin.cpp
)

if (WIN32)
	target_link_libraries(returnperiodtobin wingetopt)
endif()

install(TARGETS returnperiodtobin RUNTIME DESTINATION bin)