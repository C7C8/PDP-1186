# PDP-1186

[![Build Status](https://travis-ci.org/C7C8/PDP-1186.svg?branch=master)](https://travis-ci.org/C7C8/PDP-1186) [![codecov](https://codecov.io/gh/C7C8/PDP-1186/branch/master/graph/badge.svg)](https://codecov.io/gh/C7C8/PDP-1186)

Emulator for the PDP-11, to allow PDP-11 code to run on x86-based machines.
Because why not?

## About

The [PDP-11](https://en.wikipedia.org/wiki/PDP-11) was a 16-bit minicomputer produced by 
DEC (Digital Equipment Corporation) during the 1970's. The "mini" part is deceiving --
the actual unit was was about the size of a filing cabinet. For its time is was fairly
innovative, inspiring the design of the Intel x86 platform, which most computers today
are based on. Notably, it ran the first officially named version of Unix.

## Build

This project uses cmake. If you don't know what that is, Google it.
Doing something like `cmake .` should generate some build file stuff for
you. Here's a list of dependencies:

Build:

* None yet.

Test:

* `gtest` -- already included

