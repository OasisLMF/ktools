add_executable(fmsummaryxreftobin)
target_sources(fmsummaryxreftobin
	PRIVATE
		fmsummaryxreftobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmsummaryxreftobin wingetopt)
endif()

install(TARGETS fmsummaryxreftobin RUNTIME DESTINATION bin)