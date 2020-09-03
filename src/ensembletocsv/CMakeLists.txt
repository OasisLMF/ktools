add_executable(ensembletocsv)
target_sources(ensembletocsv
	PRIVATE
		ensembletocsv.cpp main.cpp
)
if (WIN32)
	target_link_libraries(ensembletocsv, wingetopt)
endif()
install(TARGETS ensembletocsv RUNTIME DESTINATION bin)
