add_executable(summarycalctobin)
target_sources(summarycalctobin
	PRIVATE
		summarycalctobin.cpp
)

if (WIN32)
	target_link_libraries(summarycalctobin wingetopt)
endif()

install(TARGETS summarycalctobin RUNTIME DESTINATION bin)