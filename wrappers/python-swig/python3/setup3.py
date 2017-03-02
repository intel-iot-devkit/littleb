#!/usr/bin/env python

"""
setup.py file for SWIG _littleb for python version 3.x
"""
from distutils.core import setup, Extension
import distutils.sysconfig
from distutils.command.build_ext import build_ext
from distutils.sysconfig import customize_compiler

class my_build_ext(build_ext):
    def build_extensions(self):
        customize_compiler(self.compiler)
        try:
            self.compiler.compiler_so.remove("-Wstrict-prototypes")
        except (AttributeError, ValueError):
            pass
        build_ext.build_extensions(self)

example_module = Extension('_littleb',
                           sources=['littleb_wrap.cxx', '../../../src/device.cpp', '../../../src/devicemanager.cpp', 
                           '../../../src/littlebtypes.cpp', '../../../src/littleb.c'],
                           include_dirs=["../../../include", "../../../api", "../"],
                           runtime_library_dirs=["/lib/x86_64-linux-gnu/"],
                           libraries=["systemd"],
                           extra_compile_args=['-std=c++11', '-DBUILDING_WITH_SWIG', '-DPYTHON3']
                           )

setup (name = '_littleb',
      cmdclass = {'build_ext': my_build_ext},
       version = '0.1',
       author      = "David Saper & Ron Yehudai",
       description = """littleb module in python converted with swig""",
       ext_modules = [example_module],
       py_modules = ["littleb_example"],
       )

