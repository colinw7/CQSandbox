proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  set ::cube [sb3d::shape]
  $::cube set cube {1 1 1}

  $::cube set color red

  set ::surface [sb3d::surface]

  $::surface set size {128 128}

  $::surface set position {-0.5 -0.5 0}
# $::surface set angles {-90 0 0}

  set ::shader [sb3d::shader]

# $::shader set vertex_shader   custom_shaders/surface_shader.vs
# $::shader set fragment_shader custom_shaders/surface_shader.fs

  $::shader set vertex_shader   custom_shaders/water.vs
  $::shader set fragment_shader custom_shaders/water.fs

  $::surface set cull_face 0

  $::surface set shader $::shader

  set ::elapsed 0
  $::shader set uniform_real time $::elapsed

  set waterHeight 0
  set numWaves    4

  $::shader set uniform_real waterHeight $waterHeight
  $::shader set uniform_int  numWaves $numWaves

  set amplitude_list  {}
  set wavelength_list {}
  set speed_list      {}
  set direction_list  {}

  for {set i 0} {$i < $numWaves} {incr i} {
    set amplitude  [expr {0.5/($i + 1.0)}]
    set wavelength [expr {8.0*$::PI/($i + 1.0)}]
    set speed      [expr {1.0 + 2.0*$i}]

    set angle [randIn [expr {-$::PI/3}] [expr {$::PI/3}]]
    set direction [list [expr {cos($angle)}] [expr {sin($angle)}]]

    lappend amplitude_list  $amplitude
    lappend wavelength_list $wavelength
    lappend speed_list      $speed
    lappend direction_list  $direction
  }

  $::shader set uniform_real_array amplitude    $amplitude_list
  $::shader set uniform_real_array wavelength   $wavelength_list
  $::shader set uniform_real_array speed        $speed_list
  $::shader set uniform_point2d_array direction $direction_list
}

proc tick { } {
  incr ::elapsed

  $::shader set uniform_real time [expr {$::elapsed/100.0}]
}
