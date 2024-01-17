set(MainQmlPath ${CMAKE_SOURCE_DIR}/Resource)

if (CMAKE_BUILD_TYPE MATCHES "Release")
    set(MainQmlRes qml.qrc CACHE STRING "main qml res path")
   # set(TaoQuickImport "qrc:///" CACHE STRING "main qml import path")
    set(MainImage "qrc:/Resource/" CACHE STRING "main qml image path")

else()
   # set(MainQmlRes qml.qrc CACHE STRING "main qml res path")
   # set(TaoQuickImport "file:///${MainQmlPath}" CACHE STRING "main qml import path")
    set(MainImage "file:///${MainQmlPath}/" CACHE STRING "main qml image path")
endif()

#add_compile_definitions(TaoQuickImport="${TaoQuickImport}")
add_compile_definitions(MainImage="${MainImage}")

#add_compile_definitions(TaoQuickImportPath="${TaoQuickImport}")
add_compile_definitions(MainImageResourcePath="${MainImage}")

#add_compile_definitions(QML_IMPORT_PATH="${MainQmlPath}")
#add_compile_definitions(QML2_IMPORT_PATH="${MainQmlPath}")
#add_compile_definitions(QML_DESIGNER_IMPORT_PATH="${MainQmlPath}")
