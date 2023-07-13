add_executable(amplificationstocsv)
target_sources(amplificationstocsv
        PRIVATE
                amplificationstocsv.cpp main.cpp
)

if (WIN32)
        target_link_libraries(amplificationstocsv wingetopt)
endif()

install(TARGETS amplificationstocsv RUNTIME DESTINATION bin)
