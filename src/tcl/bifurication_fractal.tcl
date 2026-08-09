proc init { } {
  #echo "init"

  # sb::canvas set window.size [list 512 512]

  set ::rmin 0.6
  set ::rmax 4.2
  set ::ymin 0.0
  set ::ymax 1.0
  set ::yset 0

  set ::min_iterations 64
  set ::max_iterations 256

  set ::renderer [sb::renderer]

  set ::feigenbaum 0
}

proc pixelXToUser { x } {
  #return [expr {(($x*1.0)/($::pixelWidth - 1))*($::xmax - $::xmin) + $::xmin}]
  return [expr {$x*$::xf + $::xmin}]
}

proc pixelYToUser { y } {
  #return [expr {(($y*1.0)/($::pixelHeight - 1))*($::ymin - $::ymax) + $::ymax}]
  return [expr {$y*$::yf + $::ymax}]
}

proc drawBg { } {
  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  if {! $::yset} {
    set rinc [expr {($::rmax - $::rmin)/99}]

    set r [expr {$::rmin + $rinc}]

    set range_set 0

    for {set px 0} {$px < 100} {incr px} {
      set y 0.5

      for {set i 0} {$i < $::max_iterations} {incr i} {
        set y [expr {$r*$y*(1.0 - $y)}]

        if {$y < -1E6 || $y > 1E6} {
          break
        }

        if {$i < $::min_iterations || $y < 0.0} {
          continue
        }

        set y1 $y

        if {$::feigenbaum} {
          set y1 [expr {$y1/$r}]
        }

        if {! $range_set} {
          set ::ymin $y1
          set ::ymax $y1

          set range_set 1
        } else {
          set ::ymin [expr {min($::ymin, $y1)}]
          set ::ymax [expr {max($::ymax, $y1)}]
        }
      }

      set r [expr {$r + $rinc}]
    }

    set ::yset 1
  }

  set rinc [expr {($::rmax - $::rmin)/($::pixelWidth - 1)}]

  set r [expr {$::rmin + $rinc}]

  for {set px 0} {$px < $::pixelWidth} {incr px} {
    set y 0.5

    for {set i 0} {$i < $::max_iterations} {incr i} {
      set y [expr {$r*$y*(1.0 - $y)}]

      if {$y < -1E6 || $y > 1E6} {
        break
      }

      if {$i < $::min_iterations || $y < 0.0} {
        continue
      }

      set y1 $y

      if {$::feigenbaum} {
        set y1 [expr {$y1/$r}]
      }

      set py [expr {int(($::pixelHeight - 1)*($::ymax - $y1)/($::ymax - $::ymin))}]

      $::renderer exec draw.point [list $px $py]
    }

    set r [expr {$r + $rinc}]
  }
}
