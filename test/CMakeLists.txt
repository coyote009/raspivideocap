set( TARGET test_raspivideocap )

cmake_minimum_required( VERSION 2.8 )

find_package( OpenCV REQUIRED )

include_directories( src /opt/raspivideocap/include )
link_directories( /opt/raspivideocap/lib /opt/vc/lib )

file( GLOB SRCS *.cpp )

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

add_executable( ${TARGET} ${SRCS} )
target_link_libraries( ${TARGET} ${MMAL_LIBS} ${OPENCV_LIBS} pthread raspivideocap )
