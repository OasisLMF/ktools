add_executable(crossvalidation)
target_sources(crossvalidation
	PRIVATE
		crossvalidation.cpp main.cpp
)

if (WIN32)
	target_link_libraries(crossvalidation wingetopt)
endif()

install(TARGETS crossvalidation RUNTIME DESTINATION bin)