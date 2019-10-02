add_executable(footprinttocsv)
target_sources(footprinttocsv
	PRIVATE
		footprinttocsv.cpp main.cpp
)

target_link_libraries(footprinttocsv ${ZLIB_LIBRARIES})
if (WIN32)
	target_link_libraries(footprinttocsv wingetopt ${ZLIB_LIBRARIES})
endif()
