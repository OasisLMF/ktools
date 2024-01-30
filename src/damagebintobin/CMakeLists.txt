add_executable(damagebintobin)
target_sources(damagebintobin
	PRIVATE
		../include/validate.h ../include/validate.cpp ../validatedamagebin/validatedamagebin.h ../validatedamagebin/validatedamagebin.cpp damagebintobin.cpp
)
if (WIN32)
	target_link_libraries(damagebintobin wingetopt)
endif()
install(TARGETS damagebintobin RUNTIME DESTINATION bin)
