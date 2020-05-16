add_executable(validatedamagebin)
target_sources(validatedamagebin
	PRIVATE
		validatedamagebin.cpp main.cpp
)

if (WIN32)
	target_link_libraries(validatedamagebin wingetopt)
endif()

install(TARGETS validatedamagebin RUNTIME DESTINATION bin)