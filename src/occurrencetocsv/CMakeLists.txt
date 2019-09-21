add_executable(occurrencetocsv)
target_sources(occurrencetocsv
	PRIVATE
		occurrencetocsv.cpp
)

if (WIN32)
	target_link_libraries(occurrencetocsv wingetopt)
endif()
