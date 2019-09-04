add_executable(footprintconv)
target_sources(footprintconv
	PRIVATE
		footprintconv.cpp
)

target_link_libraries(footprintconv ${ZLIB_LIBRARIES})
