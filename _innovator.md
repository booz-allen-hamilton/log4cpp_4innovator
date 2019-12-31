# Overview

[CMock](https://github.com/ThrowTheSwitch/CMock) and [gMock](https://github.com/google/googletest/tree/master/googlemock) are both great tools, but neither supports mocking static class member functions ... yet. The goals of this effort are to:

1.  Demonstrate using CMock and gMock together in the same test
2.  Extend those tools to mock static member functions

# Assumption(s)

* Working from ubuntu-18.04.3-desktop-amd64 VM with default configuration

# Get Code, Including Submodules
This is a fork of [log4cpp](http://log4cpp.sourceforge.net/) with CMock and gMock submodules added. The CMock changes are still a work-in-progress, so it is also a fork. However no changes were needed to use gMock. 

    git clone https://github.com/booz-allen-hamilton/log4cpp_4innovator.git --recurse-submodules
    cd log4cpp_4innovator

# Install Dependencies

### log4cpp

    sudo apt-get install build-essential
    sudo apt install autoconf

### Unity/CMock

    sudo apt install ruby-bundler

### gtest/gMock

    sudo apt install cmake

# Build log4cpp
(The thing we're testing!)

    touch aclocal.m4 configure Makefile.in Makefile.am # update timestamps
    ./configure
    make

:warning: NOTE: I was unsure how to properly generate the config/make files, so I added those from the release tarball, but then you need to freshen the timestamps (first command above) after pulling from git. Obviously this is not the best way to do things, but it works.

Optionally, you can verify the few changes to the code have not broken anything by running the pre-existing tests:

    make check

# Build gMock
(So the unit tests can link to it.)

    pushd googletest/googlemock/
    cmake .
    make
    popd


# Build and Run New Unit Tests
gMock integrates nicer with Unity so that combination (i.e., Unity + CMock + gMock) is recommended, but you can use the gtest framework instead of Unity.

    pushd tests

    # Only do once
    cat Makefile.add >> Makefile

    # Using Unity test framework
    make clean && make && make test && ./build/test/test_ct

    # Alternative using googletest framework
    rm -rf gt && make gt && ./gt

    popd

:memo: **test_ct.c** and **gt.cpp** contain different tests.

# Next Steps

While these tests demonstrate achievement of the goals, and the Unity/CMock internal tests currently pass, there remains more work to do, including:

1. Miscellaneous refactoring/cleanup
2. Pull requests into CMock and Unity master
3. Improve C++ parser and support for nested classes, multiple classes in a single header, etc.
