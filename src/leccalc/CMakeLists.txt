add_executable(leccalc)
target_sources(leccalc
	PRIVATE
		leccalc.cpp leccalc.h aggreports.cpp aggreports.h main.cpp
)

if (WIN32)
	target_link_libraries(leccalc wingetopt)
endif()
