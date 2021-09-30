add_executable(quantiletocsv)
target_sources(quantiletocsv
	PRIVATE
		quantiletocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(quantiletocsv wingetopt)
endif()

install(TARGETS quantiletocsv RUNTIME DESTINATION bin)
