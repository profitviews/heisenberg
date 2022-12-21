/* hello.i */

%module helloWorld
%{
#define SWIG_FILE_WITH_INIT
#include "helloWorld.h"
%}

extern void helloWorld();