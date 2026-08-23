set a [sb3d::astar 10 10]

set from [list 0 0]
set to   [list 9 9]

echo [$a get nx]
echo [$a get ny]

$a set empty 0 [list 5 5]
$a set empty 0 [list 6 6]

while {1} {
  set from1 [$a get next $from $to]

  echo "$from -> $to : $from1"

  set from $from1

  if {[lindex $from 0] == [lindex $to 0] && [lindex $from 1] == [lindex $to 1]} {
    break
  }
}

exit
