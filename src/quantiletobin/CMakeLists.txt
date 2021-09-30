add_executable(quantiletobin)
target_sources(quantiletobin
	PRIVATE
		quantiletobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(quantiletobin wingetopt)
endif()

install(TARGETS quantiletobin RUNTIME DESTINATION bin)
