add_executable(gultocsv)
target_sources(gultocsv
	PRIVATE
		gultocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(gultocsv wingetopt)
endif()
