set (target aalcalc)
add_executable(aalcalctocsv)
target_sources(aalcalctocsv
	PRIVATE
		aalcalctocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(aalcalctocsv wingetopt)
endif()

install(TARGETS aalcalctocsv RUNTIME DESTINATION bin)