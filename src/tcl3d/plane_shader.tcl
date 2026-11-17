proc init { } {
  set ::plane [sb3d::plane]

  set ::shader [sb3d::shader]

  $::shader set vertex_shader   custom_shaders/plane_shader.vs
  $::shader set fragment_shader custom_shaders/plane_shader.fs

  $::plane set shader $::shader

  set ::rcolor1 0
  set ::gcolor1 1
  set ::bcolor1 0

  set ::rcolor2 0
  set ::gcolor2 0
  set ::bcolor2 1

  $::shader set uniform_color xcolor [list $::rcolor1 $::gcolor1 $::bcolor1]
  $::shader set uniform_color ycolor [list $::rcolor2 $::gcolor2 $::bcolor2]

  set ::dr1 0.007
  set ::dg1 0.013
  set ::db1 0.017

  set ::dr2 0.005
  set ::dg2 0.008
  set ::db2 0.014
}

proc fix_value { v dv } {
  upvar $v v1
  upvar $dv dv1

  if {$v1 > 1.0} { set v1 1.0; set dv1 [expr {-$dv1}] }
  if {$v1 < 0.0} { set v1 0.0; set dv1 [expr {-$dv1}] }
}

proc tick { } {
  set ::rcolor1 [expr {$::rcolor1 + $::dr1}]
  set ::gcolor1 [expr {$::gcolor1 + $::dg1}]
  set ::bcolor1 [expr {$::bcolor1 + $::db1}]

  fix_value ::rcolor1 ::dr1
  fix_value ::gcolor1 ::dg1
  fix_value ::bcolor1 ::db1

  set ::rcolor2 [expr {$::rcolor2 + $::dr2}]
  set ::gcolor2 [expr {$::gcolor2 + $::dg2}]
  set ::bcolor2 [expr {$::bcolor2 + $::db2}]

  fix_value ::rcolor2 ::dr2
  fix_value ::gcolor2 ::dg2
  fix_value ::bcolor2 ::db2

  $::shader set uniform_color xcolor [list $::rcolor1 $::gcolor1 $::bcolor1]
  $::shader set uniform_color ycolor [list $::rcolor2 $::gcolor2 $::bcolor2]
}
