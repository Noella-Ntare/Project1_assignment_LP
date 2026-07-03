from distutils.core import setup, Extension

module = Extension(
    "faststats",
    sources=["faststatsmodule.c"],
    libraries=["m"],   # link libm for sqrt()
    extra_compile_args=["-O2"],
)

setup(
    name="faststats",
    version="1.0",
    description="Fast statistical computation C extension",
    ext_modules=[module],
)
