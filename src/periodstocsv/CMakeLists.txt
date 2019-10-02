add_executable(periodstocsv)
target_sources(periodstocsv
	PRIVATE
		periodstocsv.cpp
)

if (WIN32)
	target_link_libraries(periodstocsv wingetopt)
endif()
