add_executable(weightstocsv)
target_sources(weightstocsv
	PRIVATE
		weightstocsv.cpp
)

if (WIN32)
	target_link_libraries(weightstocsv wingetop)
endif()

install(TARGETS weightstocsv RUNTIME DESTINATION bin)
