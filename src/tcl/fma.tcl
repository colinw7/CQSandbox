set iter 100000

echo $tcl_version

proc test1 { a b c } {
  for {set i 0} {$i < $::iter} {incr i} {
    set res [expr {$a*$b + $c}]
  }

  echo $res
}

proc test2 { a b c } {
  for {set i 0} {$i < $::iter} {incr i} {
    set res [sb::fma $a $b $c]
  }

  echo $res
}

echo [time {test1 1.2 3.4 5.6}]
echo [time {test2 1.2 3.4 5.6}]

exit
