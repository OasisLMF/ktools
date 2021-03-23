add_executable(ordleccalc)
target_sources(ordleccalc
	PRIVATE
		../leccalc/leccalc.cpp ../leccalc/leccalc.h ../leccalc/aggreports.cpp ../leccalc/aggreports.h main.cpp
)

if (WIN32)
	target_link_libraries(ordleccalc wingetopt)
endif()

install(TARGETS ordleccalc RUNTIME DESTINATION bin)
