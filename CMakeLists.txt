project(XML_PARSER)
cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

if(NOT VCPKG_TOOLCHAIN)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()


# Boost
find_package(Boost COMPONENTS filesystem program_options serialization system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Library
set (XML_PARSER_LIB_SOURCES "src/lib/common/Base64.cpp"
                       "src/lib/Serializer.cpp"
                       "src/lib/processor/Reader.cpp"
                       "src/lib/processor/Writer.cpp"
                       "src/lib/common/Stream.cpp")
add_library(libxmlparser ${XML_PARSER_LIB_SOURCES})
target_link_libraries(libxmlparser ${Boost_LIBRARIES})

# CLI
set (XML_PARSER_CLI_SOURCES "src/bin/main.cpp")
add_executable(xmlparser ${XML_PARSER_CLI_SOURCES})
target_include_directories(xmlparser PRIVATE "src/lib")
target_link_libraries(xmlparser ${Boost_LIBRARIES})
target_link_libraries(xmlparser libxmlparser)


# Installation
#
install(
    TARGETS libxmlparser xmlparser 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib)

#
# CPack
#
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_PACKAGE_ARCHITECTURE amd64)
else()
  set(CPACK_PACKAGE_ARCHITECTURE i686)
endif()

set(CPACK_PACKAGE_NAME "bwxml")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}) 
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "WG Engine XML Parser")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCHITECTURE})
set(CPACK_PACKAGE_CONTACT "Niko Granö <niko@granö.fi>")

include(CPack)