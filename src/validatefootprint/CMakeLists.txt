add_executable(validatefootprint)
target_sources(validatefootprint
	PRIVATE
		../include/validate.h ../include/validate.cpp validatefootprint.h validatefootprint.cpp main.cpp
)

if (WIN32)
	target_link_libraries(validatefootprint wingetopt)
endif()

install(TARGETS validatefootprint RUNTIME DESTINATION bin)
