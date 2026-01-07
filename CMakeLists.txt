cmake_minimum_required(VERSION 3.10)

project(OCC-SELFDEMO LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


########### VTK ###########
set(VTK_DIR "" CACHE PATH "path to VTK library.")
find_package(VTK REQUIRED HINTS "${VTK_DIR}/lib/cmake" "${VTK_DIR}")
if(NOT VTK_FOUND)
    message(FATAL_ERROR "VTK not found. Please check the VTK_DIR.")
else()
    message(STATUS "VTK found: ${VTK_DIR}")
    message(STATUS "VTK version: ${VTK_VERSION}")
    # message(STATUS "VTK libraries: ${VTK_LIBRARIES}")
endif()

########### OCC ###########
set(OCC_DIR "" CACHE PATH "path to OpenCasCade library.")
find_package(OpenCASCADE REQUIRED HINTS "${OCC_DIR}/cmake" "${OCC_DIR}")
if(NOT OpenCASCADE_FOUND)
    message(FATAL_ERROR "OpenCASCADE not found. Please check the OCC_DIR.")
else()
    message(STATUS "OpenCASCADE found: ${OCC_DIR}")
    message(STATUS "OpenCASCADE version: ${OpenCASCADE_VERSION}")
    message(STATUS "OpenCASCADE include dirs: ${OpenCASCADE_INCLUDE_DIR}")
    # message(STATUS "OpenCASCADE libraries: ${OpenCASCADE_LIBRARIES}")
endif()

########### TBB ###########
set(TBB_DIR "" CACHE PATH "path to TBB library.")
find_package(TBB REQUIRED HINTS "${TBB_DIR}" "${TBB_DIR}/lib/cmake/tbb")
if (NOT TBB_FOUND)
    message(FATAL_ERROR "TBB library not found. Please install TBB or set the correct path.")
else()
    message(STATUS "TBB library version: ${TBB_VERSION}")
    message(STATUS "TBB library found at: ${TBB_DIR}")
endif()


find_package(OpenMP REQUIRED)

set(targetLib 
    ${OpenCASCADE_LIBRARIES} 
    ${VTK_LIBRARIES}
    TBB::tbb
    OpenMP::OpenMP_CXX
)

# set(2d_target "${CMAKE_PROJECT_NAME}_2D")
add_executable(Proj2d proj2d.cpp)
add_executable(Proj3d proj3d.cpp)
add_executable(ProjParallel ProjParallel.cpp)
add_executable(TopoNaming topoNaming.cpp)
add_executable(TopoNamingFreeCAD topoNamingFreeCAD.cpp)

#link
target_link_libraries(Proj2d ${targetLib})
target_link_libraries(Proj3d ${targetLib}) 
target_link_libraries(ProjParallel ${targetLib})
target_link_libraries(TopoNaming ${targetLib})
target_link_libraries(TopoNamingFreeCAD ${targetLib})

