add_executable(coveragetocsv)
target_sources(coveragetocsv
	PRIVATE
		coveragetocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(coveragetocsv wingetopt)
endif()

install(TARGETS coveragetocsv RUNTIME DESTINATION bin)