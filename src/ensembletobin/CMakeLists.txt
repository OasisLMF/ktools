add_executable(ensembletobin)
target_sources(ensembletobin
	PRIVATE
		ensembletobin.cpp main.cpp
)
if (WIN32)
	target_link_libraries(ensembletobin, wingetopt)
endif()
install(TARGETS ensembletobin RUNTIME DESTINATION bin)
