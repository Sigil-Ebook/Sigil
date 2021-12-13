# Pull in th proper cmake file depending on whether
# a qt5 or qt6 build was requested (defaults to qt5)

if (${USE_QT6})
    include(CMakeLists6.txt)
else()
    include(CMakeLists5.txt)
endif()
