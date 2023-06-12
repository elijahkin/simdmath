program Mandelbrot
implicit none

   ! Define parameters for Mandelbrot
   integer :: n       = 1024
   integer :: iters   = 100
   complex :: center  = (-0.5, 0)
   real    :: apothem = 1.5

   ! https://riptutorial.com/fortran/example/30514/array-operations
   complex, dimension(n,n) :: C, Z
   Z = (0, 0)

   integer :: i, j
   do i = 1,n
      do j = 1,n
         print *, i, " and ", j

end program Mandelbrot