#
# Include the TEA standard macro set
#

builtin(include,tclconfig/tcl.m4)

#------------------------------------------------------------------------
# TEA_PROG_INSTALLED_TCLSH --
#	Determine the fully qualified path name of the installed tclsh
#	executable (not the build directory version).
#
# Arguments:
#	none
#
# Results:
#	Substitutes the following vars:
#		INSTALLED_TCLSH
#------------------------------------------------------------------------

AC_DEFUN([TEA_PROG_INSTALLED_TCLSH], [
    AC_MSG_CHECKING([for installed tclsh])
    if test "${TEA_PLATFORM}" = "windows"; then
        INSTALLED_TCLSH_NAME="tclsh${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION}${EXEEXT}"
    else
        INSTALLED_TCLSH_NAME="tclsh${TCL_MAJOR_VERSION}${TCL_MINOR_VERSION}"
    fi

    # Search for installed tclsh in standard locations
    list="`ls -d ${TCL_PREFIX}/bin 2>/dev/null` \
          `ls -d ${exec_prefix}/bin 2>/dev/null` \
          `ls -d ${TCL_BIN_DIR}/../bin 2>/dev/null` \
          `ls -d ${TCL_BIN_DIR}/.. 2>/dev/null`"
    for i in $list ; do
        if test -f "$i/${INSTALLED_TCLSH_NAME}" ; then
            REAL_INSTALLED_BIN_DIR="`cd "$i"; pwd`/"
            break
        fi
    done

    if test -n "${REAL_INSTALLED_BIN_DIR}" ; then
        INSTALLED_TCLSH="${REAL_INSTALLED_BIN_DIR}${INSTALLED_TCLSH_NAME}"
    else
        # Fallback to exec_prefix/bin
        INSTALLED_TCLSH="${exec_prefix}/bin/${INSTALLED_TCLSH_NAME}"
    fi

    AC_MSG_RESULT([${INSTALLED_TCLSH}])
    AC_SUBST(INSTALLED_TCLSH)
])
