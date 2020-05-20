add_executable(fmcalc)
target_sources(fmcalc
	PRIVATE
		fmcalc.cpp fmcalc.h main.cpp fmcalcrules.cpp fmcalcdoit.cpp fmstructs.h
)
if (WIN32)
	target_link_libraries(fmcalc wingetopt)
endif()

install(TARGETS fmcalc RUNTIME DESTINATION bin)