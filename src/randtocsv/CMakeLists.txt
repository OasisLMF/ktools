add_executable(randtocsv)
target_sources(randtocsv
	PRIVATE
		randtocsv.cpp
)

if (WIN32)
	target_link_libraries(randtocsv wingetopt)
endif()
