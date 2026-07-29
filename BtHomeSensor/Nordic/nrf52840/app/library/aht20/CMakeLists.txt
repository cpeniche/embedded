set(LIBRARY_BMP "aht20")
add_library(${LIBRARY_BMP} STATIC 
  src/aht20.cpp
  )
target_include_directories(${LIBRARY_BMP} PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(${LIBRARY_BMP} PUBLIC zephyr_interface)