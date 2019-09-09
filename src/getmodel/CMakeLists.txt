add_executable(getmodel)
target_sources(getmodel
	PRIVATE
		getmodel.cpp getmodel.h main.cpp
)

target_link_libraries(getmodel z)
