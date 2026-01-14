proc read_file { f } {
  set fp [open $f r]

  set data [read $fp]

  close $fp

  return $data
}

proc init { } {
  sb3d::canvas set cull_face 0

  set ::shape [sb3d::shader_shape]

# $::shape set points     {{0.5 0.5 0.0} {0.5 -0.5 0.0} {-0.5 -0.5 0.0} {-0.5 0.5 0.0}}
# $::shape set colors     {{1.0 0.0 0.0} {0.0 1.0 0.0} {0.0 0.0 1.0} {1.0 1.0 1.0}}
# $::shape set tex_coords {{1 1} {1 0} {0 0} {0 1}}
# $::shape set indices    {0 1 3 1 2 3}
  $::shape set cube [list 0.2 0.2 0.2]
# $::shape set sphere 0.6
# $::shape set cylinder [list 0.4 0.5]
# $::shape set cone [list 0.4 0.5]

# $::shape set shader_texture [list 0 [read_file shader3d/metaballs.fs]]
  $::shape set shader_texture [list 0 [read_file shader3d/mandelbrot.fs]]
}
