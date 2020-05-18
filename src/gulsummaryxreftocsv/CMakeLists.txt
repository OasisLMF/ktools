add_executable(gulsummaryxreftocsv)
target_sources(gulsummaryxreftocsv
	PRIVATE
		gulsummaryxreftocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(gulsummaryxreftocsv wingetopt)
endif()

install(TARGETS gulsummaryxreftocsv RUNTIME DESTINATION bin)