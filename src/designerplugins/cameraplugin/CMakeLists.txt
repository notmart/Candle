find_package(Qt6 COMPONENTS Core Widgets UiPlugin Multimedia REQUIRED)

file(GLOB SRC_FILES *.cpp)

add_library(cameraplugin SHARED ${SRC_FILES})

target_link_libraries(cameraplugin PRIVATE Qt6::Core Qt6::Widgets Qt6::UiPlugin Qt6::Multimedia)

install(TARGETS cameraplugin
    RUNTIME DESTINATION ${DESIGNER_DESTINATION}
    LIBRARY DESTINATION ${DESIGNER_DESTINATION})

if(WIN32)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/ DESTINATION . FILES_MATCHING PATTERN *.dll PATTERN cameraplugin.dll EXCLUDE)
endif()
