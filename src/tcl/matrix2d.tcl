proc printMatrix { a } {
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

set ia [sb::int_matrix  3 3]
set ra [sb::real_matrix 3 3]

printMatrix $ia
printMatrix $ra

$ia set value {0 0} 1
$ia set value {1 1} 5
$ia set value {2 2} 3

echo "[$ia get value {0 0}]"
echo "[$ia get value {1 1}]"
echo "[$ia get value {2 2}]"

$ra set value {0 0} 1.1
$ra set value {1 1} 2.5
$ra set value {2 2} 3.2

echo "[$ra get value {0 0}]"
echo "[$ra get value {1 1}]"
echo "[$ra get value {2 2}]"

set ia1 [$ia get dup]
set ra1 [$ra get dup]

printMatrix $ia1
printMatrix $ra1
