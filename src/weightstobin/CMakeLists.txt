add_executable(weightstobin)
target_sources(weightstobin
	PRIVATE
		weightstobin.cpp
)

if (WIN32)
	target_link_libraries(weightstobin wingetop)
endif()

install(TARGETS weightstobin RUNTIME DESTINATION bin)
