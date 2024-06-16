cmake_minimum_required(VERSION 3.5)

project(hseqt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# torch setup
# find_package(CUDAToolkit REQUIRED PATHS C:/Qt/6.5.3/msvc2019_64/v11.8/lib/cmake/libcudacxx)
set(CMAKE_CUDA_COMPILER "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.8")
set(CMAKE_CUDA_ARCHITECTURES 60 61 62 70 72 75 86)
find_package(Torch REQUIRED PATHS C:/Qt/6.7.0/mingw_64/libtorch/share/cmake/Torch)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
include_directories(${TORCH_INCLUDE_DIRS})

# --------------------------------------------------------

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)
add_subdirectory(SQLite3/)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(hseqt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        registrationwindow.h registrationwindow.cpp registrationwindow.ui
        authwindow.h authwindow.cpp authwindow.ui
        history.h
        griddelete.h
        usersdb.h


    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET hseqt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(hseqt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(hseqt
            ${PROJECT_SOURCES}
        )
    endif()
endif()


# target_link_libraries(hseqt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets SQLite::SQLite3 "${TORCH_LIBRARIES}" CUDA::cudart)
target_link_libraries(hseqt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets sqlite3.lib "${TORCH_LIBRARIES}" CUDA::cudart)

set_property(TARGET hseqt PROPERTY CXX_STANDARD 17)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.

if (MSVC)
  file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
  add_custom_command(TARGET hseqt
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${TORCH_DLLS}
                     $<TARGET_FILE_DIR:hseqt>)
endif (MSVC)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()


if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.hseqt)
endif()
set_target_properties(hseqt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    CUDA_SEPARABLE_COMPILATION ON
)

include(GNUInstallDirs)
install(TARGETS hseqt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(hseqt)
endif()
