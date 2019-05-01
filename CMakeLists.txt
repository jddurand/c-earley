CMAKE_MINIMUM_REQUIRED (VERSION 3.0.0 FATAL_ERROR)
IF (NOT MYPACKAGEBOOTSTRAP_DONE)
  INCLUDE ("3rdparty/github/cmake-utils/cmake/MyPackageBootstrap.cmake")
ENDIF ()
#
# SET (MYPACKAGE_DEBUG TRUE)
#
MYPACKAGESTART (earley 1 0 0)

###########
# Library #
###########
MYPACKAGELIBRARY(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
  ${INCLUDE_OUTPUT_PATH}/earley/internal/config.h
  src/earley/grammar.c)

###############
# Executables #
###############

################
# Dependencies #
################
MYPACKAGEDEPENDENCY (genericLogger "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericLogger" LIBS LOCAL)
MYPACKAGEDEPENDENCY (genericStack "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/github/genericStack" LIBS LOCAL)

#########
# Tests #
#########

###########
# Install #
###########

############
# Bindings #
############

#############
# Packaging #
#############
MYPACKAGEPACK ("" "Earley parser")

#########
# Setup #
#########
MYPACKAGEPRINTSETUP()
