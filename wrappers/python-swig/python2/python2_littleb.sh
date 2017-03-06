#!/bin/bash
swig -c++  -I../ -python littleb.i
python2 setup2.py build_ext --inplace