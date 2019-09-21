add_executable(aacalctocsv)
target_sources(aacalctocsv
	PRIVATE
		aalcalctocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(aacalctocsv wingetopt)
endif()
