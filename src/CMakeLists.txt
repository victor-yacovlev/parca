cmake_minimum_required(VERSION 2.8.3)
if(WIN32)
    find_package(Boost 1.46 COMPONENTS python filesystem system REQUIRED)
else()
    find_package(Boost 1.44 COMPONENTS python REQUIRED)
endif(WIN32)
find_package(PythonInterp)
find_package(PythonLibs)
include_directories(${BOOST_INCLUDEDIR})
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR})
if (MSVC)
    set(CMAKE_CXX_FLAGS "-Zm1000 -EHsc -GR")
    set(CMAKE_MODULE_LINKER_FLAGS "/LIBPATH:${BOOST_ROOT}\\lib")
endif(MSVC)

set(HEADERS 
    a.h
    matrix.h
    pareto.h
	_parca.h
)

set(SOURCES
    a.cpp
    matrix.cpp
    pareto.cpp
    _parca.cpp
)

python_add_module(_parca ${SOURCES})
if(WIN32)
    target_link_libraries(_parca ${Boost_LIBRARIES} ${PYTHON_LIBRARY})
elseif(APPLE)
    target_link_libraries(_parca ${Boost_LIBRARIES} ${PYTHON_LIBRARY})
else()
    target_link_libraries(_parca ${Boost_LIBRARIES})
endif(WIN32)

if(APPLE)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "import sys; sys.stdout.write('/Library/Python/'+sys.version[0:3]+'/site-packages/parca')"
        OUTPUT_VARIABLE PYTHON_INSTALL_DIR
    )
elseif(WIN32)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "import sys; import sysconfig; sys.stdout.write(sysconfig.get_path('platlib')+'/parca')"
        OUTPUT_VARIABLE PYTHON_INSTALL_DIR
    )
else()
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c "import sys; import sysconfig; sys.stdout.write(sysconfig.get_path('platlib').replace(sys.prefix,'${CMAKE_INSTALL_PREFIX}')+'/parca')"
        OUTPUT_VARIABLE PYTHON_INSTALL_DIR
    )
endif(APPLE)

message("Platform-dependent python modules will be installed in "${PYTHON_INSTALL_DIR})

install(TARGETS _parca LIBRARY DESTINATION ${PYTHON_INSTALL_DIR})
