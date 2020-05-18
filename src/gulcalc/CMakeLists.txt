add_executable(gulcalc)
target_sources(gulcalc
	PRIVATE
		gulcalc.cpp gulcalc.h getrands.cpp getrands.h primetest.cpp main.cpp doit.cpp 
)

if (WIN32)
	target_link_libraries(gulcalc wingetopt)
endif()

install(TARGETS gulcalc RUNTIME DESTINATION bin)