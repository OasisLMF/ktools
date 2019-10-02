add_executable(periodstobin)
target_sources(periodstobin
	PRIVATE
		periodstobin.cpp
)
if (WIN32)
	target_link_libraries(periodstobin wingetopt)
endif()
