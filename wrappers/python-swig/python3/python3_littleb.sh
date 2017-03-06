#!/bin/bash
swig -c++  -I../ -python littleb.i
python3 setup3.py build_ext --inplace