proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  sb::canvas set buffered 1

  sb::canvas set blend.enabled 1
  sb::canvas set blend.factor  0.95

  sb::canvas set brush.color black

  sb::canvas set range [list -4.0 -2.0 4.0 3.0]

  sb::canvas set gravity 0

  set ::t1 [sb::image {0 0} "textures/particle2.png"]
  set ::t2 [sb::image {0 0} "textures/explode.png"]

  $::t1 set scale   {0.4 0.4}
  $::t1 set visible 0
  $::t2 set visible 0

  set ::t2s 0.2

  # injection data
  set ::inject_ticks 10 ; # time to next particle

  set ::velocityXMin -2.00 ; # x velocity min/max
  set ::velocityXMax  2.00
  set ::velocityYMin  2.00 ; # y velocity min/max
  set ::velocityYMax  5.00

  # explode data
  set ::explode_ticks     80 ; # time to explode
  set ::explosion_ticks1  80 ; # explosion frame 1
  set ::explosion_ticks2  85 ; # explosion frame 2
  set ::explosion_ticks3  90 ; # explosion frame 3
  set ::explosion_ticks4  95 ; # explosion frame 4
  set ::explosion_ticks5 100 ; # explosion frame 5
  set ::explosion_ticks6 105 ; # explosion frame 6
  set ::explosion_ticks7 110 ; # explosion frame 7
  set ::explosion_ticks8 115 ; # explosion frame 8
  set ::explosion_ticks  120 ; # explosion end

  # current state
  set ::tick_count   0
  set ::inject_count 0

  set ::ox 0
  set ::oy -2.0

  set ::oy1 [expr {$::oy - 0.1}]

  sb::canvas set play 1
}

proc explodeVelocity { a } {
  set r  2.0
  set dr [expr {$r/10}]

  set vx [expr {$r*cos($a)}]
  set vy [expr {$r*sin($a)}]

  set vx1 [expr {$vx - $dr}]
  set vx2 [expr {$vx + $dr}]
  set vy1 [expr {$vy - $dr}]
  set vy2 [expr {$vy + $dr}]

  return [list [randIn $vx1 $vx2] [randIn $vy1 $vy2] 0.0]
}

proc explodeParticle { p } {
  # echo "explodeParticle"

  set pos [$p get position]

  set r [randIn 0.6 0.8]
  set g [randIn 0.6 0.8]
  set b [randIn 0.6 0.8]

  # create explosion particles
  set pl {}

  for {set i 0} {$i < 8} {incr i} {
    set p [sb::particle $pos]

    $p set meta  "explode"
    $p set age   [expr $::explode_ticks + 1]
    $p set color [list $r $g $b]

    set angle [expr {$i*45}]

    # set velocity in random directions
    $p set velocity [explodeVelocity $angle]]

    $p set image $::t2
    $p set size  0.5
    $p set angle $angle
    $p set tpos  [list 0.0 0.3333]
    $p set tsize [list 0.2 0.3333]

    lappend pl $p
  }

if {0} {
  foreach p $pl {
    echo [$p get velocity]
  }
}
}

proc injectParticle { } {
  # echo "injectParticle"

  set p [sb::particle [list $::ox $::oy 0]]

  set vx [randIn $::velocityXMin $::velocityXMax]
  set vy [randIn $::velocityYMin $::velocityYMax]

  $p set velocity [list $vx $vy 0.0]

  set r [randIn 0.5 1.0]
  set g [randIn 0.5 1.0]
  set b [randIn 0.5 1.0]

  $p set meta  "inject"
  $p set color [list $r $g $b]
  $p set age   0
  $p set image $::t1
  $p set size  0.2
}

# remap value (0 - 1) to range (vmin - vmax)
proc remap { v vmin vmax } {
  return [expr {($v - $vmin)/($vmax - $vmin) - 0.5}]
}

proc update { args } {
  # echo "update"

  incr ::tick_count

  if {$::inject_count == 0} {
    injectParticle
  }

  incr ::inject_count

  if {$::inject_count > $::inject_ticks} {
    set ::inject_count 0
  }

  foreach particle [sb::canvas get particles] {
    set dead [$particle get dead]
    if {$dead} { continue }

    set pos [$particle get position]

    if {[lindex $pos 1] <= $::oy1} {
      $particle set dead 1
      continue
    }

    set age [$particle get age]

    set meta [$particle get meta]

    if {$meta == "inject"} {
      if       {$age == $::explode_ticks} {
        explodeParticle $particle

        $particle set dead 1
      } else {
        set age1 [expr {$age + 1.0}]

        $particle set age $age1
      }
    } else {
      set age1 [expr {$age + 1.0}]

      $particle set age $age1

      set alpha [expr {($::explosion_ticks - $age1)/($::explosion_ticks - $::explode_ticks)}]
      $particle set alpha $alpha

      if       {$age1 > $::explosion_ticks} {
        $particle set dead 1
      } elseif {$age1 >= $::explosion_ticks8} {
        $particle set tpos [list 0.8 0.6666]
        $particle set size 0.80
      } elseif {$age1 >= $::explosion_ticks7} {
        $particle set tpos [list 0.6 0.6666]
        $particle set size 0.70
      } elseif {$age1 >= $::explosion_ticks6} {
        $particle set tpos [list 0.4 0.6666]
        $particle set size 0.60
      } elseif {$age1 >= $::explosion_ticks5} {
        $particle set tpos [list 0.2 0.6666]
        $particle set size 0.50
      } elseif {$age1 >= $::explosion_ticks4} {
        $particle set tpos [list 0.8 0.3333]
        $particle set size 0.40
      } elseif {$age1 >= $::explosion_ticks3} {
        $particle set tpos [list 0.6 0.3333]
        $particle set size 0.40
      } elseif {$age1 >= $::explosion_ticks2} {
        $particle set tpos [list 0.4 0.3333]
        $particle set size 0.20
      } elseif {$age1 >= $::explosion_ticks1} {
        $particle set tpos [list 0.2 0.3333]
        $particle set size 0.10
      }
    }
  }
}
