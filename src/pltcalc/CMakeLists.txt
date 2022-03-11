add_executable(pltcalc)

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
        target_sources(pltcalc
                PRIVATE
                        pltcalc.cpp main.cpp ../include/useparquet.cpp ../include/useparquet.h
        )
        target_link_libraries(pltcalc PRIVATE arrow_shared parquet_shared)
else()
        target_sources(pltcalc
                PRIVATE
                        pltcalc.cpp main.cpp
        )
endif()

if (WIN32)
	target_link_libraries(pltcalc wingetopt)
endif()

install(TARGETS pltcalc RUNTIME DESTINATION bin)
