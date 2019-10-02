add_executable(aalcalctocsv)
target_sources(aalcalctocsv
	PRIVATE
		aalcalctocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(aalcalctocsv wingetopt)
endif()
