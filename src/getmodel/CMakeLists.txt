add_executable(getmodel)
target_sources(getmodel
	PRIVATE
		getmodel.cpp getmodel.h main.cpp
)

target_link_libraries(getmodel ${ZLIB_LIBRARIES})
if (WIN32)
	target_link_libraries(getmodel wingetopt ${ZLIB_LIBRARIES})
endif()

install(TARGETS getmodel RUNTIME DESTINATION bin)