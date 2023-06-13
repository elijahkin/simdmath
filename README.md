# The intersection of parallel computing and fractals!

This repository contains my implementions of `mandelbrot` and `domain_color` which make use of the [Intel intrinsics](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html) available in C++. Essentially, these enable element-wise operations on arrays to happen concurrently. This is especially helpful for fractal generation, which involves lots of such operations. We get further speed improvements on this front by utilizing multithreading.

Currently, the tools support rendering the Mandelbrot set, as well as plotting domain colorings of functions $f: \mathbb{C} \to \mathbb{C}$. A personal favorite of mine is the function $e^{1/z}$, which has an essential singularity at $0$, as shown below.

<p align="center">
  <img src="singularity.png" width="50%" margin=auto>
</p>

After doing all the math, I currently export to .png file, however, I eventually plan to learn OpenGL and make zoomable, pannable fractals.

### What I'm Working On

* Trying to learn [OpenGL](https://learnopengl.com/Getting-started/OpenGL)
* Possibly writing a port of `simd_mandelbrot.cpp` that uses `float` instead of `double`
* Fixing `simd_domain_color.cpp` (Currently very buggy!) Maybe I should make the primitive functions return memcopies of their arguments?

### References

* Certain intrinsics used in this repository (the SVML intrinsics) are currently only compatible with the `icc` compiler, which can be installed [here](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html).
* Installing [OpenGL](https://en.wikibooks.org/wiki/OpenGL_Programming/Installation/Mac) on macOS