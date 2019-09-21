add_executable(randtobin)
target_sources(randtobin
	PRIVATE
		randtobin.cpp
)

if (WIN32)
	target_link_libraries(randtobin wingetopt)
endif()
