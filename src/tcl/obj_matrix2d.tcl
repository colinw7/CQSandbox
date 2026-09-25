proc printMatrix { a id } {
  set dim1 [$a get dim1]
  set dim2 [$a get dim2]

  echo "$id\[$dim1,$dim2\]"

  echo -nonewline "\{"
  for {set i 0} {$i < $dim1} {incr i} {
    if {$i > 0} { echo -nonewline " " }

    echo -nonewline "\{"
    for {set j 0} {$j < $dim2} {incr j} {
      if {$j > 0} { echo -nonewline " " }

      if {[$a get has_value $i $j]} {
        echo -nonewline "[$a get value $i $j]"
      } else {
        echo -nonewline "null"
      }
    }

    echo -nonewline "\}"
  }
  echo "\}"
}

set a [sb::obj_matrix 3 4]

$a set value 0 0 1
$a set value 1 1 1.4
$a set value 2 2 "Hello"
$a set value 2 3 {1 2}

printMatrix $a "a"

echo "[$a get value 0 0]"
echo "[$a get value 1 1]"
echo "[$a get value 2 2]"
echo "[$a get value 2 3]"
