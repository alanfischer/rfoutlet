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
add_executable (rfoutlet ${SRC})
set_target_properties (rfoutlet PROPERTIES COMPILE_DEFINITIONS "RFOUTLET_MAIN")
install (TARGETS rfoutlet DESTINATION bin)

# Build library
add_library (librfoutlet ${SRC})
set_target_properties (librfoutlet PROPERTIES OUTPUT_NAME "rfoutlet")
install (TARGETS librfoutlet DESTINATION lib)
install (FILES ${HDR} DESTINATION include)

# Find the mongoose library
find_path (MONGOOSE_INCLUDE_DIR mongoose.h)
if(MONGOOSE_INCLUDE_DIR)
    include_directories (${MONGOOSE_INCLUDE_DIR})

    set (REST_SRC ${SRC} rest.cpp ${MONGOOSE_INCLUDE_DIR}/mongoose.c)

    add_executable (restrfoutlet ${REST_SRC})
    install (TARGETS restrfoutlet DESTINATION bin)
else()
    message("mongoose not found, not building rest api")
endif()
