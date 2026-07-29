set(LIBRARY_BMP "bmp280")
add_library(${LIBRARY_BMP} STATIC 
  src/bmp280.c
  )
target_include_directories(${LIBRARY_BMP} PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries(${LIBRARY_BMP} PUBLIC zephyr_interface)

