add_executable(amplificationstobin)
target_sources(amplificationstobin
        PRIVATE
                amplificationstobin.cpp main.cpp
)

if (WIN32)
        target_link_libraries(amplificationstobin wingetopt)
endif()

install(TARGETS amplificationstobin RUNTIME DESTINATION bin)
