add_library(drivers STATIC 
  src/nrfPwm.cpp
  src/nrfI2c.cpp
  src/bluetooth.c)
add_dependencies(drivers zephyr_generated_headers)
target_include_directories(drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(drivers PUBLIC zephyr_interface)