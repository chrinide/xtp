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
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/715944016/ctp_run.o \
	${OBJECTDIR}/_ext/715944016/ctp_run2.o


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
LDLIBSOPTIONS=../libctp/dist/Debug/GNU-Linux-x86/liblibctp.a ../../../csg/netbeans/libcsg/../../src/libcsg/libcsg.a ../../../moo/netbeans/libmoo/../../src/libmoo/libmoo.a ../../../tools/netbeans/libtools/../../src/libtools/libtools.a -lgmx -lexpat -lsqlite3 -lboost_program_options -lpthread -lm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../src/tools/ctp_run

../../src/tools/ctp_run: ../libctp/dist/Debug/GNU-Linux-x86/liblibctp.a

../../src/tools/ctp_run: ../../../csg/netbeans/libcsg/../../src/libcsg/libcsg.a

../../src/tools/ctp_run: ../../../moo/netbeans/libmoo/../../src/libmoo/libmoo.a

../../src/tools/ctp_run: ../../../tools/netbeans/libtools/../../src/libtools/libtools.a

../../src/tools/ctp_run: ${OBJECTFILES}
	${MKDIR} -p ../../src/tools
	${LINK.cc} -o ../../src/tools/ctp_run ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/715944016/ctp_run.o: ../../src/tools/ctp_run.cc 
	${MKDIR} -p ${OBJECTDIR}/_ext/715944016
	${RM} $@.d
	$(COMPILE.cc) -g -I../../include -I../../../tools/include -I../../../moo/include -I../../../csg/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/715944016/ctp_run.o ../../src/tools/ctp_run.cc

${OBJECTDIR}/_ext/715944016/ctp_run2.o: ../../src/tools/ctp_run2.cc 
	${MKDIR} -p ${OBJECTDIR}/_ext/715944016
	${RM} $@.d
	$(COMPILE.cc) -g -I../../include -I../../../tools/include -I../../../moo/include -I../../../csg/include -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/715944016/ctp_run2.o ../../src/tools/ctp_run2.cc

# Subprojects
.build-subprojects:
	cd ../libctp && ${MAKE}  -f Makefile CONF=Debug
	cd ../../../csg/netbeans/libcsg && ${MAKE}  -f Makefile_nb CONF=Debug
	cd ../../../moo/netbeans/libmoo && ${MAKE}  -f Makefile_nb CONF=Debug
	cd ../../../tools/netbeans/libtools && ${MAKE}  -f Makefile_nb CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../src/tools/ctp_run

# Subprojects
.clean-subprojects:
	cd ../libctp && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../../csg/netbeans/libcsg && ${MAKE}  -f Makefile_nb CONF=Debug clean
	cd ../../../moo/netbeans/libmoo && ${MAKE}  -f Makefile_nb CONF=Debug clean
	cd ../../../tools/netbeans/libtools && ${MAKE}  -f Makefile_nb CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
