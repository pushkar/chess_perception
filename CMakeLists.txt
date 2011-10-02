cmake_minimum_required (VERSION 2.6)
project (chess_perception)

SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_NAME "chess_perception")
SET(CPACK_SYSTEM_NAME "i386")

set (CMAKE_VERBOSE_MAKEFILE OFF)

set ( HEADERS
./
./gl
./log
./ach
/usr/include
/usr/local/include
)

set ( SRC
knn
main
chess
ach/craftyd
ach/chessd
ach/chessdout
sensors/mesa
log/aio_reader
log/aio_writer
log/logread
log/logwrite
gl/glcamera
gl/glstdlib
gl/view
)

find_package(GLUT)
find_package(OpenGL)
find_package ( OpenCV REQUIRED )

include_directories (${HEADERS} ${OpenCV_INCLUDE_DIRS} )
add_executable (chess_perception ${SRC})
target_link_libraries (chess_perception ach mesasr somatic amino ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${OPENCV_LIBRARY} opencv_highgui)

