This repository centers around my implemention of the `SimdComplex` class, which uses vector instrinsics to speed up element-wise computations on arrays of complex numbers. I use this class for many different types of fractal generations.

The `SimdComplex` class uses SVML intrinsics that are currently only available by using the `icc` compiler, which can be installed [here](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html).

Further, I would like to thank Sean Barrett for his [stb] image writing tool, which I use to export the fractals as .png images.