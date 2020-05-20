add_executable(validateoasisfiles)
target_sources(validateoasisfiles
	PRIVATE
		validateoasisfiles.cpp main.cpp
)

if (WIN32)
	target_link_libraries(validateoasisfiles wingetopt)
endif()

install(TARGETS validateoasisfiles RUNTIME DESTINATION bin)