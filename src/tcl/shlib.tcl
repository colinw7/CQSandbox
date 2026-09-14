proc init { } {
  sb::canvas set module_dir modules/mandelbrot

  set mandelbrot [sb::shlib mandelbrot]

  echo [$mandelbrot get xmin]
  echo [$mandelbrot get ymin]
  echo [$mandelbrot get xmax]
  echo [$mandelbrot get ymax]

  $mandelbrot set a 1

  echo [$mandelbrot exec calc 0.1 0.2]

  exit
}
