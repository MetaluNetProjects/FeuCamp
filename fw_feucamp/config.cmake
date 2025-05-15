target_link_libraries(${CMAKE_PROJECT_NAME} 
    ws2812
    audio
    pico_sync
    pico_multicore
    pico_rand
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hw)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

