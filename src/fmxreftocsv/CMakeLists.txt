add_executable(fmxreftocsv)
target_sources(fmxreftocsv
	PRIVATE
		fmxreftocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmxreftocsv wingetopt)
endif()
