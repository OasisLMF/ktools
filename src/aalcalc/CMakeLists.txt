add_executable(aalcalc "")
target_sources(aalcalc
	PRIVATE
		aalcalc.cpp aalcalc.h main.cpp summaryindex.cpp waalcalc.cpp
)

if (WIN32)
	target_link_libraries(aalcalc wingetopt)
endif()
