add_executable(fmpolicytctobin)
target_sources(fmpolicytctobin
	PRIVATE
		fmpolicytctobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmpolicytctobin wingetopt)
endif()

install(TARGETS fmpolicytctobin RUNTIME DESTINATION bin)