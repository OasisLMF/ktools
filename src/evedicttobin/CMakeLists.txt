add_executable(evedicttobin)
target_sources(evedicttobin
	PRIVATE
		evedicttobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(evedicttobin wingetopt)
endif()
install(TARGETS evedicttobin RUNTIME DESTINATION bin)
