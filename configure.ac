#!/bin/bash -norc
dnl	This file is an input file used by the GNU "autoconf" program to
dnl	generate the file "configure", which is run during Tcl installation
dnl	to configure the system for the local environment.

#-----------------------------------------------------------------------
# Package information
#-----------------------------------------------------------------------

AC_INIT([ntcan],[1.3.0])

#-----------------------------------------------------------------------
# TEA initialization
#-----------------------------------------------------------------------

TEA_INIT()

#-----------------------------------------------------------------------
# C++ support (C++11)
#-----------------------------------------------------------------------

TEA_INIT_CXX([11])

#-----------------------------------------------------------------------
# Set auxiliary directory
#-----------------------------------------------------------------------

AC_CONFIG_AUX_DIR(tclconfig)

# Generate config.h header file for configuration results
AC_CONFIG_HEADERS([config.h])

#--------------------------------------------------------------------
# Load the tclConfig.sh file
#-----------------------------------------------------------------------

TEA_PATH_TCLCONFIG
TEA_LOAD_TCLCONFIG

#-----------------------------------------------------------------------
# Load the tkConfig.sh file if necessary (commented out for now)
#-----------------------------------------------------------------------

#TEA_PATH_TKCONFIG
#TEA_LOAD_TKCONFIG

#-----------------------------------------------------------------------
# Handle the --prefix=... option by defaulting to what Tcl gave.
# Must be called after TEA_LOAD_TCLCONFIG and before TEA_SETUP_COMPILER.
#-----------------------------------------------------------------------

TEA_PREFIX

#-----------------------------------------------------------------------
# Standard compiler checks.
# This sets up CC by using the CC env var, or looks for gcc otherwise.
# This also calls AC_PROG_CC, AC_PROG_INSTALL and a few others to create
# the basic setup necessary to compile executables.
#-----------------------------------------------------------------------

TEA_SETUP_COMPILER

#-----------------------------------------------------------------------
# Specify the C/C++ source files and any other subsidiary files
# that are involved in the build.
#-----------------------------------------------------------------------

TEA_ADD_SOURCES([generic/ntcan.cpp])
TEA_ADD_HEADERS([])
TEA_ADD_INCLUDES([])
TEA_ADD_LIBS([-lntcan])
TEA_ADD_CFLAGS([])
TEA_ADD_STUB_SOURCES([])
TEA_ADD_TCL_SOURCES([])

#-----------------------------------------------------------------------
# Choose which headers you need. Extension authors should try very
# hard to only rely on the Tcl public header files. Internal headers
# contain private data structures and are subject to change without
# notice.
# This MUST be called after TEA_LOAD_TCLCONFIG / TEA_LOAD_TKCONFIG
#-----------------------------------------------------------------------

TEA_PUBLIC_TCL_HEADERS
#TEA_PRIVATE_TCL_HEADERS
#TEA_PUBLIC_TK_HEADERS
#TEA_PRIVATE_TK_HEADERS
#TEA_PATH_X

#-----------------------------------------------------------------------
# Check whether --enable-threads or --disable-threads was given.
#-----------------------------------------------------------------------

TEA_ENABLE_THREADS

#-----------------------------------------------------------------------
# The statement below defines a collection of symbols related to
# building as a shared library instead of a static library.
#-----------------------------------------------------------------------

TEA_ENABLE_SHARED

#-----------------------------------------------------------------------
# This macro figures out what flags to use with the compiler/linker
# when building shared/static debug/optimized objects. This information
# can be taken from the tclConfig.sh file, but this figures it all out.
#-----------------------------------------------------------------------

TEA_CONFIG_CFLAGS

#-----------------------------------------------------------------------
# Set the default compiler switches based on the --enable-symbols option.
#-----------------------------------------------------------------------

TEA_ENABLE_SYMBOLS

#-----------------------------------------------------------------------
# This macro generates a line to use when building a library. It
# depends on values set by the TEA_ENABLE_SHARED, TEA_ENABLE_SYMBOLS,
# and TEA_LOAD_TCLCONFIG macros above.
#-----------------------------------------------------------------------

TEA_MAKE_LIB

#-----------------------------------------------------------------------
# Find tclsh so that we can run pkg_mkIndex to create the pkgIndex.tcl
# file during the install process. Don't run the TCLSH_PROG through
# ${CYGPATH} because it's being used directly by make.
# Require that we use a tclsh shell version 8.2 or later since earlier
# versions have bugs in the pkg_mkIndex routine.
# Add WISH as well if this is a Tk extension.
#-----------------------------------------------------------------------

TEA_PROG_TCLSH
TEA_PROG_INSTALLED_TCLSH
#TEA_PROG_WISH

#-----------------------------------------------------------------------
# Finally, substitute all of the various values into the Makefile.
# You may alternatively have a special pkgIndex.tcl.in or other files
# which require substituting the AC variables in. Include these here.
#-----------------------------------------------------------------------

AC_CONFIG_FILES([Makefile pkgIndex.tcl])
AC_OUTPUT
