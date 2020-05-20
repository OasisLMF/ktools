add_executable(fmsummaryxreftocsv)
target_sources(fmsummaryxreftocsv
	PRIVATE
		fmsummaryxreftocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmsummaryxreftocsv wingetopt)
endif()

install(TARGETS fmsummaryxreftocsv RUNTIME DESTINATION bin)