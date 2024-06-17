cmake_minimum_required (VERSION 3.5)

project(Scanner VERSION 1.0
	DESCRIPTION "Verdi project"
	LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_DOC "Build documentation" ON)

## START RAVENRB
set (RAVENCPP_SRC "${PROJECT_SOURCE_DIR}/libs/RavenDB" CACHE INTERNAL "RAVENCPP_SRC")

#fetch RavenDB client from a repo
include(FetchContent)
FetchContent_Declare(
  ravendb_client_cpp
  GIT_REPOSITORY https://github.com/rerunner/ravendb-cpp-client.git
  GIT_TAG        master
  SOURCE_DIR ${RAVENCPP_SRC}/repository SUBBUILD_DIR ${RAVENCPP_SRC}/subbuild  BINARY_DIR ${RAVENCPP_SRC}/binary
  )

FetchContent_GetProperties(ravendb_client_cpp)
if(NOT ravendb_client_cpp_POPULATED)  
  FetchContent_Populate(ravendb_client_cpp)

  #since we want only the client, we don't want to compile tests and tryouts, only the client
  set(BUILD_TRYOUTS OFF)
  set(BUILD_TESTS OFF)
  add_subdirectory("${RAVENCPP_SRC}/repository/" "${RAVENCPP_SRC}/binary")
endif() 

list(APPEND CMAKE_INCLUDE_PATH ${RAVENCPP_SRC}/repository/Raven.CppClient)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE INTERNAL "CMAKE_INCLUDE_PATH")

message("Raven.CppDemo include: ${RAVENCPP_SRC}/repository/Raven.CppClient")
message("CMAKE_INCLUDE_PATH : ${CMAKE_INCLUDE_PATH}")
message("CMAKE_LIBRARY_PATH : ${CMAKE_LIBRARY_PATH}")

include_directories(${CMAKE_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})
## END RAVENRB


find_package(CURL) 
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(Doxygen)

find_library(BOOST_SYSTEM boost_system)

find_library(HIBERLITE hiberlite)

find_library(CURL_LIBRARIES curl)

set(CXXFLAGS  "-fdiagnostics-color=always -g -std=c++20 -DSTRING_NAMES=1 -DL1D_CACHE_LINE_SIZE=64")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

set(src "${CMAKE_CURRENT_SOURCE_DIR}")
set(dst "${CMAKE_CURRENT_BINARY_DIR}")
set(all_targets VerdiTest)

#Some files need to be moved into the temporary destination folder
#foreach(file Scanner.idl rtps.ini run_all.sh)
#	configure_file(${src}/${file} ${dst}/${file} COPYONLY)
#endforeach()

if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${src}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${dst}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)


# Scanner target
add_executable(VerdiTest
	${src}/include/Uuid.cpp
	${src}/src/main.cpp
)

target_include_directories(VerdiTest PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${RAVENCPP_SRC}/repository/External/xxHash/repository" "${src}/src" "${src}/src" "${src}/include")
target_link_libraries(VerdiTest Raven_CppClient_static nlohmann_json::nlohmann_json xxhash ${BOOST_SYSTEM} ${HIBERLITE} ${CURL_LIBRARIES})

