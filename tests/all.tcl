# all.tcl --
#
#	This file runs all of the tests in the rsvisa test suite.
#
# Usage: tclsh all.tcl [-verbose [level]]
#
# This script is a standard template for Tcl test packages.

if {[lsearch [namespace children] ::tcltest] == -1} {
    package require tcltest
    namespace import ::tcltest::*
}

set ::tcltest::testSingleFile false
set ::tcltest::testsDirectory [file dir [info script]]

# We need to ensure that the libraries are found.
if {[info exists ::env(TCLLIBPATH)]} {
    set old_tcllibpath $::env(TCLLIBPATH)
} else {
    set old_tcllibpath {}
}

set ::env(TCLLIBPATH) [list [file join [file dir [file dir [info script]]]] \
    {*}$old_tcllibpath]

puts stdout "Tests running in interp:       [info nameofexecutable]"
puts stdout "Tests running with pwd:        [pwd]"
puts stdout "Tests running in working dir:  $::tcltest::testsDirectory"
if {[llength $::env(TCLLIBPATH)] > 0} {
    puts stdout "Package search path:          $::env(TCLLIBPATH)"
}

if {[llength $::tcltest::skip] > 0} {
    puts stdout "Skipping tests that match:            $::tcltest::skip"
}
if {[llength $::tcltest::match] > 0} {
    puts stdout "Only running tests that match:        $::tcltest::match"
}

if {[llength $::tcltest::skipFiles] > 0} {
    puts stdout "Skipping test files that match:       $::tcltest::skipFiles"
}
if {[llength $::tcltest::matchFiles] > 0} {
    puts stdout "Only sourcing test files that match:  $::tcltest::matchFiles"
}

set timeCmd {clock format [clock seconds]}
puts stdout "Tests began at [eval $timeCmd]"
puts stdout ""

# Source each of the specified tests
foreach file [lsort [::tcltest::getMatchingFiles]] {
    set tail [file tail $file]
    puts stdout $tail
    if {[catch {source $file} msg]} {
        puts stdout "Test file error: $msg"
        # append errors so they can be printed in a summary later
        append ::tcltest::errors($tail) $msg
    }
}

# cleanup
puts stdout "\nTests ended at [eval $timeCmd]"
::tcltest::cleanupTests 1
return
