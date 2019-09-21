add_executable(itemtobin)
target_sources(itemtobin
	PRIVATE
		itemtobin.cpp main.cpp
)

if (WIN32)
	target_link_libraries(itemtobin wingetopt)
endif()
