proc init { } {
  set ::particles [sb3d::particle_list]

  set ::num_particles 2000

  $::particles set size $::num_particles

  set ::t  0
  set ::dt [expr {1.0/60.0}]

  updateParticles
}

proc update { } {
  set ::t [expr {$::t + $::dt}]

  updateParticles
}

proc updateParticles { } {
  #puts $::t

  set w 1
  set h 1

  for {set i 0} {$i < $::num_particles} {incr i} {
    set i1 [expr {$i + 1}]

    set ri [expr {(1.0*$i1)/1000.0}]

    set r [expr {min(99*$i1, 255)/255.0}]
    set g [expr {min( 2*$i1, 255)/255.0}]
    set b [expr {min(   $i1, 255)/255.0}]

    $::particles set color [list $r $g $b] $i

    set F [expr {0.2*($::t + 9.0)/$ri + sin($ri*$ri)}]

    set x [expr {$w/2.0 +          $ri*sin($F)}]
    set y [expr {$h/2.0 + 0.2*(2.0*$ri*cos($F) + 1.0*(0.001*$::num_particles)/(1.0*$ri))}]
    set z $ri
   #set r [expr {sin($ri)*9.0}]

    #puts "$x $y $z"
    $::particles set position [list $x $y $z] $i
  }
}
