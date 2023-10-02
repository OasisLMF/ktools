add_executable(aalcalcmeanonly)
target_sources(aalcalcmeanonly
	PRIVATE
		aalcalcmeanonly.cpp main.cpp
)
if (WIN32)
	target_link_libraries(aalcalcmeanonly wingetopt)
endif()

install(TARGETS aalcalcmeanonly RUNTIME DESTINATION bin)
