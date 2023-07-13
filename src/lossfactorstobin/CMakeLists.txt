add_executable(lossfactorstobin)
target_sources(lossfactorstobin
	PRIVATE
		lossfactorstobin.cpp main.cpp
)

if (WIN32)
	target_link_libraries(lossfactorstobin wingetopt)
endif()

install(TARGETS lossfactorstobin RUNTIME DESTINATION bin)
