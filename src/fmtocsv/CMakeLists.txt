add_executable(fmtocsv)
target_sources(fmtocsv
	PRIVATE
		fmtocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmtocsv wingetopt)
endif()
