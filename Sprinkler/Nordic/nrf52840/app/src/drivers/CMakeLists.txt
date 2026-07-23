add_library(drivers STATIC src/nrfPwm.cpp)
target_include_directories(drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(drivers PUBLIC zephyr_interface)