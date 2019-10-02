add_executable(occurrencetobin)
target_sources(occurrencetobin
	PRIVATE
		occurrencetobin.cpp
)

if (WIN32)
	target_link_libraries(occurrencetobin wingetopt)
endif()
