find_package(Qt6 COMPONENTS Core Widgets UiPlugin REQUIRED)

file(GLOB SRC_FILES *.cpp)

add_library(customwidgetsplugin SHARED ${SRC_FILES})

target_include_directories(customwidgetsplugin PRIVATE ../../customwidgets)

target_link_libraries(customwidgetsplugin PRIVATE customwidgets Qt6::Core Qt6::Widgets Qt6::UiPlugin)

install(TARGETS customwidgetsplugin
    RUNTIME DESTINATION ${DESIGNER_DESTINATION}
    LIBRARY DESTINATION ${DESIGNER_DESTINATION})
