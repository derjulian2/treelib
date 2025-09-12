
from setuptools import setup, Extension

setup(
    name="fib",
    version="0.69",
    description="fibonacci number computation in C",
    ext_modules=[Extension("fib", sources=["fib.c"], include_dirs=["C:/msys64/ucrt64/include/python3.12"])]
)
