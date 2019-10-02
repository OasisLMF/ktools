add_executable(fmprofiletocsv)
target_sources(fmprofiletocsv
	PRIVATE
		fmprofiletocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(fmprofiletocsv wingetopt)
endif()
