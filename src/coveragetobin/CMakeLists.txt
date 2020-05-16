add_executable(coveragetobin)
target_sources(coveragetobin
	PRIVATE
		coveragetobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(coveragetobin wingetopt)
endif()

install(TARGETS coveragetobin RUNTIME DESTINATION bin)