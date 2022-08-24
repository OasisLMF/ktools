add_executable(evedicttocsv)
target_sources(evedicttocsv
	PRIVATE
		evedicttocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(evedicttocsv wingetopt)
endif()
install(TARGETS evedicttocsv RUNTIME DESTINATION bin)
