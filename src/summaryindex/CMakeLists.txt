add_executable(summaryindex)
target_sources(summaryindex
	PRIVATE
		summaryindex.cpp main.cpp
)

if (WIN32)
	target_link_libraries(summaryindex wingetopt)
endif()
