#!/bin/bash

pushd ..
premake5 --cc=gcc --file=Build.lua ninja
popd
