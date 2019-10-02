add_executable(gultobin)
target_sources(gultobin
	PRIVATE
		gultobin.cpp main.cpp
)

if (WIN32)
	target_link_libraries(gultobin wingetopt)
endif()
