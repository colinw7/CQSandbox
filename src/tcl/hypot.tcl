set iter 10000

echo $tcl_version

proc test1 { x y } {
  for {set i 0} {$i < $::iter} {incr i} {
    set res [expr {sqrt($x*$x + $y*$y)}]
  }

  echo $res
}

proc test2 { x y } {
  for {set i 0} {$i < $::iter} {incr i} {
    set res [sb::hypot $x $y]
  }

  echo $res
}

echo [time {test1 3 4}]
echo [time {test2 3 4}]

exit
