add_executable(summarycalctocsv)
target_sources(summarycalctocsv
	PRIVATE
		summarycalctocsv.cpp
)

if (WIN32)
	target_link_libraries(summarycalctocsv wingetopt)
endif()
