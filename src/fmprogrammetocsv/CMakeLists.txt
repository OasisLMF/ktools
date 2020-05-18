add_executable(fmprogrammetocsv)
target_sources(fmprogrammetocsv
	PRIVATE
		fmprogrammetocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmprogrammetocsv wingetopt)
endif()

install(TARGETS fmprogrammetocsv RUNTIME DESTINATION bin)