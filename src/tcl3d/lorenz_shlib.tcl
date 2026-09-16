proc remap { v vmin vmax } {
  return [expr {($v - $vmin)/($vmax - $vmin) - 0.5}]
}

proc nextColor { } {
  incr ::color_num

  if {$::color_num >= [llength $::colors]} {
    set ::color_num 0
  } 
}

proc init { } {
  sb3d::canvas set module_dir modules/lorenz

  set ::lorenz [sb3d::shlib lorenz]

  #---

  set ::in 0

  set ::n 50000
 #set ::n 8000
 #set ::n 800

  set ix 0.0
  set iy 1.0
  set iz 0.0

  set dt 0.01

  set ::A { 10.0 }
  set ::B { 28.0 }
  set ::C [expr { 8.0/3.0 }]

  set ::rk_t 0.0
  set ::rk_x 0.0
  set ::rk_y 1.0
  set ::rk_z 0.0

  set ::rk_dt 0.01

  initColors

  # set t1 [sb3d::image {0 0} "textures/particle1.png"]

  # $t1 set scale {0.4 0.4}

  # $t1 set visible 0

  set ::particles [sb3d::particle_list]

  $::particles set size $::n

  set ::xmin -17.99
  set ::ymin -24.15
  set ::zmin   0.00
  set ::xmax  19.83
  set ::ymax  27.64
  set ::zmax  48.31

  $::particles set particle.size 0.007

  echo [time {calcNext}]
}

proc initColors { } {
  set ::colors {}
  set ::color_num 0

  if {0} {
    lappend ::colors [getAppValue color "green"]
    lappend ::colors [getAppValue color "blue"]
    lappend ::colors [getAppValue color "cyan"]
    lappend ::colors [getAppValue color "magenta"]
    lappend ::colors [getAppValue color "yellow"]
    lappend ::colors [getAppValue color "orange"]
    lappend ::colors [getAppValue color "brown"]
    lappend ::colors [getAppValue color "grey50"]
    lappend ::colors [getAppValue color "pink"]
    lappend ::colors [getAppValue color "purple"]
    lappend ::colors [getAppValue color "darkgreen"]
    lappend ::colors [getAppValue color "lightblue"]
    lappend ::colors [getAppValue color "seagreen"]
    lappend ::colors [getAppValue color "lightsteelblue"]
    lappend ::colors [getAppValue color "tan"]
    lappend ::colors [getAppValue color "black"]
    lappend ::colors [getAppValue color "white"]
  } else {
    set nc 32
    for {set i 0} {$i < $nc} {incr i} {
      lappend ::colors [list [expr {(1.0*$i)/$nc}] 0 0]
    }
    for {set i 0} {$i < $nc} {incr i} {
      lappend ::colors [list 0 [expr {(1.0*$i)/$nc}] 0]
    }
    for {set i 0} {$i < $nc} {incr i} {
      lappend ::colors [list 0 0 [expr {(1.0*$i)/$nc}]]
    }
  }
}

proc calcNext { } {
  set x1 0.0
  set y1 0.0
  set z1 0.0

  for {set i 0} {$i < $::n} {incr i} {
    set pos [$::lorenz exec calc]

    if {$i >= $::in} {
      set x [remap [lindex $pos 0] $::xmin $::xmax]
      set y [remap [lindex $pos 1] $::ymin $::ymax]
      set z [remap [lindex $pos 2] $::zmin $::zmax]

      # echo "$x $y $x"

      $::particles set position [list $x $y $z] $i

      if {$x1*$x < 0 || $y1*$y < 0 || $z1*$z < 0} {
        nextColor
      }

      set color [lindex $::colors $::color_num]

      $::particles set color $color $i
  #   $p set image $t1

      set x1 $x
      set y1 $y
      set z1 $z
    }
  }
}
