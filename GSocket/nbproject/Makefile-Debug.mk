#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/GSocket.o \
	${OBJECTDIR}/src/GSocketClient.o \
	${OBJECTDIR}/src/GSocketServer.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk libgsocket.a

libgsocket.a: ${OBJECTFILES}
	${RM} libgsocket.a
	${AR} -rv libgsocket.a ${OBJECTFILES} 
	$(RANLIB) libgsocket.a

${OBJECTDIR}/src/GSocket.o: src/GSocket.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../ByteBuffer -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/GSocket.o src/GSocket.cpp

${OBJECTDIR}/src/GSocketClient.o: src/GSocketClient.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../ByteBuffer -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/GSocketClient.o src/GSocketClient.cpp

${OBJECTDIR}/src/GSocketServer.o: src/GSocketServer.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../ByteBuffer -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/GSocketServer.o src/GSocketServer.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
