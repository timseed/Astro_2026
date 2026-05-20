cmake_minimum_required(VERSION 3.15.0)
project(gsc_project)

##################  Paths  ################################
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/GSC/")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(APPLE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit")
endif(APPLE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

set(util_SRCS prtgsc.c dispos.c decode_c.c get_head.c find_reg.c to_d.c dtos.c)

######################################
################ GSC #################
######################################
add_executable(gsc gsc.c ${util_SRCS})
target_link_libraries(gsc m)
install(TARGETS gsc RUNTIME DESTINATION bin)
################################################
#################### Decode ####################
################################################
add_executable(decode decode.c decode_c.c get_head.c prtgsc.c dtos.c)
target_link_libraries(decode m)
install(TARGETS decode RUNTIME DESTINATION bin)
################################################
#################### Encode ####################
################################################
add_executable(encode encode.c get_head.c prtgsc.c dtos.c)
target_link_libraries(encode m)
install(TARGETS encode RUNTIME DESTINATION bin)
################################################
#################### epoch ####################
################################################
add_executable(epoch epoch.c get_head.c prtgsc.c dtos.c)
target_link_libraries(epoch m)
install(TARGETS epoch RUNTIME DESTINATION bin)
######################################
################ genreg  #################
######################################
add_executable(genreg genreg.c ${util_SRCS})
target_link_libraries(genreg m)
install(TARGETS genreg RUNTIME DESTINATION bin)
######################################
################ gsc_dec  #################
######################################
add_executable(gsc_dec gsc_dec.c ${util_SRCS})
target_link_libraries(gsc_dec m)
install(TARGETS gsc_dec RUNTIME DESTINATION bin)
######################################
################ gsc_enc  #################
######################################
#add_executable(gsc_enc gsc_enc.c ${util_SRCS})
#target_link_libraries(gsc_enc m)
#install(TARGETS gsc_enc RUNTIME DESTINATION bin)

###################################
## Install Files and Directories ##
###################################

install(FILES regions.bin regions.ind DESTINATION ${DATA_INSTALL_DIR}/bin)

install(DIRECTORY N0000 N1500 N3000 N4500 N6000 N7500 N0730 N2230 N3730 N5230 N6730 N8230 S0000 S1500 S3000 S4500 S6000 S7500 S0730 S2230 S3730 S5230 S6730 S8230 DESTINATION ${DATA_INSTALL_DIR})

