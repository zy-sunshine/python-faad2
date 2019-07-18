from distutils.core import setup, Extension

faad2mod = Extension('faad2',
    libraries = ['faad'],
    sources = ['faad2mod.c'])
setup(name = 'faad2 python library',
    version = '1.0',
    description = 'This is python wrap of faad2 library',
    ext_modules = [faad2mod])
