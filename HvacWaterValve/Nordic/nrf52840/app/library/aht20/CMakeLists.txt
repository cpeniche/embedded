set(LIBRARY_AHT20 "aht20")
add_library(${LIBRARY_AHT20} STATIC 
  src/aht20.cpp
  )

add_dependencies(${LIBRARY_AHT20} zephyr_generated_headers)
target_include_directories(${LIBRARY_AHT20} PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(${LIBRARY_AHT20} PUBLIC zephyr_interface)