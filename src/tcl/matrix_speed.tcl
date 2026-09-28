# 973675
# 493062

proc execMatrix { a } {
  set dim0 [$a get dim0]
  set dim1 [$a get dim1]

  set sum 0

  for {set i 0} {$i < $dim0} {incr i} {
    for {set j 0} {$j < $dim1} {incr j} {
      set sum [expr {$sum + [$a get value $i $j]}]
    }
  }

  return $sum
}

set a [sb::real_matrix 1000 1000]

echo [time {execMatrix $a}]

exit
