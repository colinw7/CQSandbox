set iter 10000

echo $tcl_version

proc test1 { } {
  set r 1.1

  for {set i 0} {$i < $::iter} {incr i} {
    set r1 [expr {$i*$i}]
  }

  echo $r1
}

proc test2 { } {
  set r 1.1

  for {set i 0} {$i < $::iter} {incr i} {
    set r1 [sb::fmul $i $i]
  }

  echo $r1
}

echo [time test1]
echo [time test2]

exit
