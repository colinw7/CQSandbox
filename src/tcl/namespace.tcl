namespace eval Vector {
  variable x 0
  variable y 0

  proc add { this n } {
    variable x
    variable y

    set x [expr {$x + $n}]
    set y [expr {$y + $n}]
  }

  proc print { args } {
    echo "$args"
  }

  proc getPrint { } {
    return [namespace code {print}]
  }
}

namespace eval Vector1 {
  variable x 1
  variable y 2
}

namespace eval Vector2 {
  variable x 3
  variable y 4
}


eval [Vector::getPrint] 1

#Vector::print

#Vector::add 5

#Vector::print
