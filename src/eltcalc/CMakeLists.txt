add_executable(eltcalc)

# Known bug where ParquetConfig.cmake is not found - workaround is to find
# directory that contains ArrowConfig.cmake and search this directory for
# ParquetConfig.cmake. See https://issues.apache.org/jira/browse/ARROW-12175
# for details.
find_package(Arrow)
if (Arrow_FOUND)
        get_filename_component(MY_SEARCH_DIR ${Arrow_CONFIG} DIRECTORY)
        find_package(Parquet HINTS ${MY_SEARCH_DIR})
endif()
if (Parquet_FOUND)
        add_definitions(-DHAVE_PARQUET)
        target_sources(eltcalc
                PRIVATE
                        eltcalc.cpp main.cpp ../include/oasisparquet.cpp ../include/oasisparquet.h
        )
        target_link_libraries(eltcalc PRIVATE arrow_shared parquet_shared)
else()
        target_sources(eltcalc
                PRIVATE
                        eltcalc.cpp main.cpp
        )
endif()

if (WIN32)
	target_link_libraries(eltcalc wingetopt)
endif()

install(TARGETS eltcalc RUNTIME DESTINATION bin)
