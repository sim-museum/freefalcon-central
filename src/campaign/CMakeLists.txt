# Campaign system
add_subdirectory(camplib)
add_subdirectory(camptask)
add_subdirectory(campui)
add_subdirectory(campupd)
if(WIN32)
    add_subdirectory(camptool)
endif()
