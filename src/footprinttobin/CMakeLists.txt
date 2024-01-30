add_executable(footprinttobin)
target_sources(footprinttobin
	PRIVATE
		../include/validate.h ../include/validate.cpp ../validatefootprint/validatefootprint.h ../validatefootprint/validatefootprint.cpp footprinttobin.h footprinttobin.cpp main.cpp
)

target_link_libraries(footprinttobin ${ZLIB_LIBRARIES})
if (WIN32)
	target_link_libraries(footprinttobin wingetopt ${ZLIB_LIBRARIES})
endif()

install(TARGETS footprinttobin RUNTIME DESTINATION bin)
