proc printArray { a } {
  set dim0 [$a get dim0]
  set dim1 [$a get dim1]

  echo -nonewline "{"
  for {set i 0} {$i < $dim0} {incr i} {
    if {$i > 0} { echo -nonewline " " }
    echo -nonewline "{"
    for {set j 0} {$j < $dim1} {incr j} {
      if {$j > 0} { echo -nonewline " " }
      echo -nonewline "[$a get value [list $i $j]]"
    }
    echo -nonewline "}"
  }
  echo "}"
}

set a [sb3d::array 3 3]

printArray $a

$a set value 1 {0 0}
$a set value 2 {1 1}
$a set value 3 {2 2}

echo "[$a get value {0 0}]"
echo "[$a get value {1 1}]"
echo "[$a get value {2 2}]"
echo "[$a get value {3 3}]"

printArray $a

set a1 [$a get dup]

printArray $a1
