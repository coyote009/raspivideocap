set( TARGET raspivideocap )
set( INSTALL_DIR /opt/raspivideocap )

cmake_minimum_required( VERSION 2.8 )

find_package( OpenCV 3 REQUIRED )

include_directories( src /opt/vc/include )
link_directories( /opt/vc/lib )

file( GLOB SRCS src/*.cpp src/*.c )

# to enable debug mode (!!Note: Clean build directory before changing mode)
#  > cmake -Ddebug=1 ..
if( debug )
  set( CMAKE_BUILD_TYPE Debug )
else()
  set( CMAKE_BUILD_TYPE Release )
endif()

set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native" )
#set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -mcpu=native -mtune=native -v" )
#set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG -mcpu=cortex-a53 -mfpu=neon-fp-armv8" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3" )

set( MMAL_LIBS mmal_core mmal mmal_util vcos bcm_host )
set( OPENCV_LIBS opencv_core opencv_imgproc opencv_highgui )

add_library( ${TARGET} SHARED ${SRCS} )
target_link_libraries( ${TARGET} ${MMAL_LIBS} ${OPENCV_LIBS} pthread )

install( TARGETS ${TARGET} LIBRARY DESTINATION ${INSTALL_DIR}/lib )
install( FILES src/raspivideocap.h DESTINATION ${INSTALL_DIR}/include )
install( SCRIPT post_install.cmake )
