add_executable(cdftocsv)
target_sources(cdftocsv
	PRIVATE
		cdftocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(cdftocsv wingetopt)
endif()
