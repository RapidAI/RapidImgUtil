cmake_minimum_required(VERSION 3.10)
project(qmimgutil)

if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")

endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


if(WIN32)


	#add_definitions(-D_BPOCR_EXPORT_API)

	SET(WINSDKROOT E:/thirdpart)

	IF(CMAKE_CL_64)
		set(platform x64)
		#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x64/)
		link_directories(${WINSDKROOT}/lib/x64)
		link_directories(${WINSDKROOT}/FreeImage/Dist/x64)
		include_directories(${WINSDKROOT}/FreeImage/Dist/x64)
		


	ELSE(CMAKE_CL_64)
		set(platform x86)
		link_directories(${WINSDKROOT}/FreeImage/Dist/x32)
		include_directories(${WINSDKROOT}/FreeImage/Dist/x32)
		#link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x86/)

	ENDIF(CMAKE_CL_64)

	if(EXISTS ${WINSDKROOT}/opencv/build)
			set(OpenCV_DIR ${WINSDKROOT}/opencv/build)
	endif()
		
	#STRING(REPLACE "-O2" "-Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

elseif(UNIX)
	

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


#set(OpenCV_STATIC 1)

FIND_PACKAGE(OpenCV REQUIRED)  # important
if(OPENCV_FOUND)
	message("OpenCV FOUND")
	include_directories(${OpenCV_INCLUDE_DIRS})
	link_directories(${OpenCV_LIBRARY_DIRS})
	link_libraries(${OpenCV_LIBS})
		
endif()


set(LIBSRC sources/rpimgutil.cpp sources/rpimagesize.cpp)



add_library(rpimgutil ${LIBSRC})

target_link_libraries(rpimgutil freeimage)

if(WIN32)
set(CMAKE_INSTALL_DIR ${WINSDKROOT})
set(LIBDIR ${CMAKE_INSTALL_DIR}/lib/x64)
else()
set(CMAKE_INSTALL_DIR /opt/qmcds/)
set(LIBDIR ${CMAKE_INSTALL_DIR}/lib)
endif()




add_executable(imgtest sources/test.cpp )
target_link_libraries(imgtest rpimgutil)

#set(CMAKE_DEBUG_POSTFIX "_d")

#set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "_d")
# install files.
INSTALL(TARGETS rpimgutil
       DESTINATION  ${LIBDIR}
      
)

install(FILES include/rpimgutil.h
		DESTINATION ${CMAKE_INSTALL_DIR}/include
)