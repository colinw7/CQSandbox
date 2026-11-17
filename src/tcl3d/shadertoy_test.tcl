proc read_file { f } {
  set fp [open $f r]

  set data [read $fp]

  close $fp

  return $data
}

proc init { } {
  sb3d::canvas set mouse.tracking 1

  set ::shader [sb3d::shader_toy]

# $::shader set fragment_shader [read_file shader3d/test.fs]
# $::shader set fragment_shader [read_file shader3d/test_color.fs]
# $::shader set fragment_shader [read_file shader3d/test_shape.fs]
# $::shader set fragment_shader [read_file shader3d/test_matrix.fs]
# $::shader set fragment_shader [read_file shader3d/test_pattern.fs]
# $::shader set fragment_shader [read_file shader3d/test_random.fs]
# $::shader set fragment_shader [read_file shader3d/test_noise.fs]
# $::shader set fragment_shader [read_file shader3d/test_cellular.fs]
  $::shader set fragment_shader [read_file shader3d/test_fractal.fs]
}
