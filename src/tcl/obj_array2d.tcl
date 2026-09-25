proc printArray { a } {
  set dim [$a get dim]

  echo -nonewline "{"
  for {set i 0} {$i < $dim} {incr i} {
    if {$i > 0} { echo -nonewline " " }
    echo -nonewline "[$a get value $i]"
  }
  echo "}"
}

set a [sb::obj_array 4]

$a set value 0 1
$a set value 1 1.4
$a set value 2 "Hello"
$a set value 3 {1 2}

printArray $a

echo "[$a get value 0]"
echo "[$a get value 1]"
echo "[$a get value 2]"
echo "[$a get value 3]"
