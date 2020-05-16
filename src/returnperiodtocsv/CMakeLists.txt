add_executable(returnperiodtocsv)
target_sources(returnperiodtocsv
	PRIVATE
		returnperiodtocsv.cpp
)

if (WIN32)
	target_link_libraries(returnperiodtocsv wingetopt)
endif()

install(TARGETS returnperiodtocsv RUNTIME DESTINATION bin)