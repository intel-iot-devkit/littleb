LittleB Wrappers

-Python - 2.x, 3.x versions.
	Built using Swig

-Javascript
	Built using nbind - https://github.com/charto/nbind

For first time use and if any changes were made to the littleB module make the wrappers runnign these:
	./make_libs.sh (in python folder)
	./make_lib.sh( in nbind folder - will run async_example.js as well)


Examples:
	-In python folder go to python3/2, you will find example run files such as unit_testing.py, sensor_example.py.
	-In the nbind folder you will see example.js, async_example.js, both examples work and show the nbinds capabilities.