add_executable(damagebintocsv)
target_sources(damagebintocsv
	PRIVATE
		damagebintocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(damagebintocsv wingetopt)
endif()

install(TARGETS damagebintocsv RUNTIME DESTINATION bin)