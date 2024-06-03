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

find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Doxygen)

find_library(RAFT_LIB raft)
find_library(RAFT_SHM shm)
find_library(RAFT_RT rt)
find_library(RAFT_AFFINITY affinity)
find_library(RAFT_DEMANGLE demangle)
find_library(RAFT_CMDARGS cmdargs)

find_library(BOOST_SYSTEM boost_system)

find_library(HIBERLITE hiberlite)

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

target_include_directories(VerdiTest PUBLIC "/usr/local/include" "/usr/include/hiberlite" "${src}/src" "${src}/include")
target_link_libraries(VerdiTest nlohmann_json::nlohmann_json ${RAFT_LIB} ${RAFT_SHM} ${RAFT_RT} ${RAFT_AFFINITY} ${RAFT_DEMANGLE} ${RAFT_CMDARGS} ${BOOST_SYSTEM} ${HIBERLITE} )

