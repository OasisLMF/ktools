add_executable(fmprogrammetobin)
target_sources(fmprogrammetobin
	PRIVATE
		fmprogrammetobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmprogrammetobin wingetopt)
endif()
