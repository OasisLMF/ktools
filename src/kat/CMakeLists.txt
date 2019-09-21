add_executable(kat)
target_sources(kat
	PRIVATE
		kat.cpp
)

if (WIN32)
	target_link_libraries(kat wingetopt)
endif()
