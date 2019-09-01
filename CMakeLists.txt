cmake_minimum_required(VERSION 3.14)
project(untitled2 C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled2 jam.c jam.h class.c excep.c resolve.c alloc.c string.c thread.c execute.c thread.h alloc.h hash.h utf8.c lock.c lock.h jni.c jni.h sig.h dll.c hash.c cast.c native.c)