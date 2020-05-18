add_executable(gulsummaryxreftobin)
target_sources(gulsummaryxreftobin
	PRIVATE
		gulsummaryxreftobin.cpp main.cpp
)

if (WIN32)
	target_link_libraries(gulsummaryxreftobin wingetopt)
endif()

install(TARGETS gulsummaryxreftobin RUNTIME DESTINATION bin)