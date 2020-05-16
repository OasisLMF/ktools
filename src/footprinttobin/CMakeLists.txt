add_executable(footprinttobin)
target_sources(footprinttobin
	PRIVATE
		footprinttobin.cpp main.cpp
)

target_link_libraries(footprinttobin ${ZLIB_LIBRARIES})
if (WIN32)
	target_link_libraries(footprinttobin wingetopt ${ZLIB_LIBRARIES})
endif()

install(TARGETS footprinttobin RUNTIME DESTINATION bin)