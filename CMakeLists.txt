# Standard Setup
cmake_minimum_required (VERSION 2.8)
project (rfoutlet CXX C)

# Includes
include_directories (source)

# Source files
set (SRC
    source/RFOutlet.cpp
)

# Headers
set (HDR
    source/RFOutlet.h
)

# Build application
set (EXE_NAME ${CMAKE_PROJECT_NAME})
add_executable (${EXE_NAME} ${SRC})
set_target_properties (${EXE_NAME} PROPERTIES COMPILE_DEFINITIONS "RFOUTLET_MAIN")
install (TARGETS ${EXE_NAME} DESTINATION bin)

# Build library
set (LIB_NAME lib${CMAKE_PROJECT_NAME})
add_library (${LIB_NAME} SHARED ${SRC})
set_target_properties (${LIB_NAME} PROPERTIES OUTPUT_NAME "rfoutlet")
install (TARGETS ${LIB_NAME} DESTINATION lib)
install (FILES ${HDR} DESTINATION include)

# Find the mongoose library
find_path (MONGOOSE_INCLUDE_DIR mongoose.h)
if(MONGOOSE_INCLUDE_DIR)
    include_directories (${MONGOOSE_INCLUDE_DIR})

    set (REST_SRC ${SRC} source/rest.cpp ${MONGOOSE_INCLUDE_DIR}/mongoose.c)

    set (RESTEXE_NAME rest${CMAKE_PROJECT_NAME})
    add_executable (${RESTEXE_NAME} ${REST_SRC})
    install (TARGETS ${RESTEXE_NAME} DESTINATION bin)
else()
    message("mongoose not found, not building rest api")
endif()
