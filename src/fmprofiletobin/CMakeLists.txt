add_executable(fmprofiletobin)
target_sources(fmprofiletobin
	PRIVATE
		fmprofiletobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmprofiletobin wingetopt)
endif()
