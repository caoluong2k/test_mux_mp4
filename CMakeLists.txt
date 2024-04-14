cmake_minimum_required(VERSION 3.16.3)
project(mp4lib VERSION 1.0)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package( Threads REQUIRED )

add_executable(test_mp4_mux_frame 
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/test_mp4_mux_frame.c
)
target_include_directories(test_mp4_mux_frame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minimp4/include)
target_link_libraries(test_mp4_mux_frame PRIVATE minimp4 h264reader)


add_executable(test_mp4_mux_audio_video ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/test_mux_audio_video.c)
target_include_directories(test_mp4_mux_audio_video PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/minimp4/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/codec_sim
    )

if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(test_mp4_mux_audio_video PUBLIC "-pthread")
endif()
target_link_libraries(test_mp4_mux_audio_video PRIVATE minimp4 h264reader codec_ipc_sim)




add_subdirectory(thirdparty)