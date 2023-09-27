add_executable(lossfactorstocsv)
target_sources(lossfactorstocsv
	PRIVATE
		lossfactorstocsv.cpp main.cpp
)

if (WIN32)
	target_link_libraries(lossfactorstocsv wingetopt)
endif()

install(TARGETS lossfactorstocsv RUNTIME DESTINATION bin)
