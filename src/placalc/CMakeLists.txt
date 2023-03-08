add_executable(placalc)
target_sources(placalc
	PRIVATE
		placalc.cpp main.cpp
)

if (WIN32)
	target_link_libraries(placlac wingetopt)
endif()

install(TARGETS gulcalc RUNTIME DESTINATION bin)
