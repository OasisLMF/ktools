add_executable(itemtocsv)
target_sources(itemtocsv
	PRIVATE
		itemtocsv.cpp
)

if (WIN32)
	target_link_libraries(itemtocsv wingetopt)
endif()
