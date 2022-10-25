file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
add_executable(3532 ${SOURCES})


target_link_libraries(3532 
easylogger
libmpi.a
libhdmi.a
libVoiceEngine.a
libaec.a
libanr.a
libresampler.a
libtde.a
libjpeg.a
libpciv.a
pthread
dl
rt
m
)

if(${CHIP_TYPE} EQUAL 3)
    install(TARGETS 3532 RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../bin/3)
else()
    install(TARGETS 3532 RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../bin/1)
endif()