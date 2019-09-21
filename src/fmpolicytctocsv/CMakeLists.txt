add_executable(fmpolicytctocsv)
target_sources(fmpolicytctocsv
	PRIVATE
		fmpolicytctocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmpolicytctocsv wingetopt)
endif()
