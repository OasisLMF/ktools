add_executable(fmprofilebinconv)
target_sources(fmprofilebinconv
	PRIVATE
		fmprofilebinconv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmprofilebinconv wingetopt)
endif()
