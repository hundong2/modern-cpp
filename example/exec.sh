#!/bin/bash
g++ ${1} -std=c++20 -o c.out -fcoroutines
./c.out
