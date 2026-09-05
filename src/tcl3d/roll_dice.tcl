proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  set model_dir "/home/colinw/dev/blender"
  sb3d::canvas set model_dir $model_dir

  set ::dice [sb3d::model "/home/colinw/dev/blender/dice.obj"]

  $::dice exec translate [list 0 -1 0]

  echo [$::dice get bbox.center]

  set ::x_angle [randIn -10 10]
  set ::y_angle [randIn -10 10]
  set ::z_angle [randIn -10 10]

  sb3d::canvas set loop.enabled 1
}

proc update { args } {
  $::dice exec rotate [list 1 0 0] $::x_angle
  $::dice exec rotate [list 0 1 0] $::y_angle
  $::dice exec rotate [list 0 0 1] $::z_angle

  set ::x_angle [expr {$::x_angle + 1}]
  set ::y_angle [expr {$::y_angle + 1}]
  set ::z_angle [expr {$::z_angle + 1}]
}
