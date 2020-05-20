add_executable(damagebintobin)
target_sources(damagebintobin
	PRIVATE
		damagebintobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(damagebintobin wingetopt)
endif()
install(TARGETS damagebintobin RUNTIME DESTINATION bin)