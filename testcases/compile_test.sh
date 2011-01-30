#!/bin/sh

echo "This needs a path to schneeflocke"

export SF_PATH=/home/nosc/diplom/schneeflocke/libschnee

echo "Compile Test 1"
sfautoreflect/sfautoreflect ../src/samples/DataSharingElements.h -o DataSharingElements_autoreflect.cpp
g++ -I../src/samples/ -I../src/ -I$SF_PATH -c DataSharingElements_autoreflect.cpp -o DataSharingElements_autoreflect.o

echo "Compile Test 2"
sfautoreflect/sfautoreflect ../src/samples/Error.h -o Error_autoreflect.cpp
g++ -I../src/samples/ -I../src/ -I$SF_PATH -c Error_autoreflect.cpp -o Error_autoreflect.o

