"""
  @author Elijah Kin
  @version 1.0
  @date 06/12/23
"""

from matplotlib import animation
from matplotlib import cm
import matplotlib.pyplot as plt
import numpy as np
import time

def mandelbrot_iters(center, apothem, iters, n):
 """Performs iteration on a rectangular region of the complex plane."""
 imag = np.linspace(center.imag - apothem, center.imag + apothem, n)
 real = np.linspace(center.real - apothem, center.real + apothem, n)
 imag = -1j * imag

 c = imag[:, np.newaxis] + real
 z = np.zeros(c.shape, dtype=np.complex128)
 escape_iter = np.zeros(c.shape, dtype=int)

 for i in range(iters):
   z *= z
   z += c

   escaped = np.absolute(z) > 2
   c[escaped] = 0
   z[escaped] = 0
   escape_iter[escaped] = i

  #  TODO Linear interpolation
  #  log_zn = np.log(z.real * z.real + z.imag * z.imag) / 2
  #  nu = np.log(log_zn / np.log(2)) / np.log(2)
  #  escape_iter[escaped] = i + 1 - nu

 return escape_iter

def plot_mandelbrot(iters, n):
 """Plots the Mandelbrot set."""
 plt.imshow(mandelbrot_iters(-0.6, 1.5, iters, n), cmap='magma')

start = time.time()
plot_mandelbrot(200, 8192)
end = time.time()
print(end - start)
plt.show()