add_executable(eve)
target_sources(eve
	PRIVATE
		eve.cpp main.cpp
)
if (WIN32)
	target_link_libraries(eve wingetopt)
endif()

install(TARGETS eve RUNTIME DESTINATION bin)