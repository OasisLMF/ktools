add_executable(aalcalcsimple)
target_sources(aalcalcsimple
	PRIVATE
		aalcalcsimple.cpp main.cpp
)
if (WIN32)
	target_link_libraries(aalcalcsimple wingetopt)
endif()

install(TARGETS aalcalcsimple RUNTIME DESTINATION bin)
