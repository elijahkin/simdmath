# The intersection of parallel computing and fractals!

This repository contains my implementions of `mandelbrot` and `domain_color` which make use of the [Intel intrinsics](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html) available in C++. Essentially, these enable element-wise operations on arrays to happen concurrently. This is especially helpful for fractal generation, which involves lots of such operations.

Certain intrinsics used in this repository (the SVML intrinsics) are currently only compatible with the `icc` compiler, which can be installed [here](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html).

Currently, the tools support rendering the Mandelbrot set, as well as plotting domain colorings of functions $f: \mathbb{C} \to \mathbb{C}$. A personal favorite of mine is the function $e^{1/z}$, which has an essential singularity at $0$, as shown below.

<p align="center">
  <img src="singularity.png" width="50%" margin=auto>
</p>

After doing all the math, I currently export to .png file using an [image writer](https://github.com/nothings/stb/blob/master/stb_image_write.h) written by Sean Barrett. However, I eventually plan to learn OpenGL and make the fractals zoomable, pannable fractals.

### Currently Working On...

* [Learning OpenGL](https://learnopengl.com/Getting-started/OpenGL)
* Possibly using [multithreading](https://www.geeksforgeeks.org/multithreading-in-cpp/) or [MPI](https://people.math.sc.edu/Burkardt/cpp_src/mpi/mpi.html). In my case, where code would only be running on one machine, multithreading seems to make more sense.
* Possibly outsourcing the math to Fortran
* Fixing `simd_domain_color.cpp` (Currently very buggy!) Maybe I should make the primitive functions return memcopies of their arguments?

### References

* Installing [OpenGL]([OpenGL](https://en.wikibooks.org/wiki/OpenGL_Programming/Installation/Mac)) on macOS