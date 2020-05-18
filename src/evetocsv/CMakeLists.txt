add_executable(evetocsv)
target_sources(evetocsv
	PRIVATE
		evetocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(evetocsv wingetopt)
endif()

install(TARGETS evetocsv RUNTIME DESTINATION bin)