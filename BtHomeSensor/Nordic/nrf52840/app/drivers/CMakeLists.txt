set(LIBRARY "drivers")
add_library(${LIBRARY} STATIC 
  src/nrfI2c.cpp
  src/bluetooth.c
  src/zephyrSpiWrapper.cpp)
add_dependencies(${LIBRARY} zephyr_generated_headers)
target_include_directories(${LIBRARY} PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(${LIBRARY} PUBLIC zephyr_interface)
