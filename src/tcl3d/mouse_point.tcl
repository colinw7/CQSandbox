proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc mapReal { r min1 max1 min2 max2 } {
  return [expr {($max2 - $min2)*($r - $min1)/($max1 - $min1) + $min2}]
}

proc init { } {
# set ::point_list1 [sb3d::point_list]
  set ::point_list2 [sb3d::point_list]

  set ::line_list [sb3d::line_list]

# $::point_list1 set point.size 0.5
  $::point_list2 set point.size 1

# $::point_list1 set visible 0
  $::point_list2 set visible 0

  $::line_list set visible 0

  $::line_list set line.width 4

  set ::sphere [sb3d::shape]
  $::sphere set sphere 0.5

  set earthmap [sb3d::texture "textures/earthmap1k.jpg"]
  $earthmap set name "earthmap"

  $::sphere set texture $earthmap
}

proc gamePress { x y obj } {
# $::point_list1 set visible 1
  $::point_list2 set visible 1
  $::line_list   set visible 1

  updatePoints $x $y
}

proc gameMove { x y } {
  updatePoints $x $y
}

proc gameRelease { x y obj } {
# $::point_list1 set visible 0
  $::point_list2 set visible 0
  $::line_list   set visible 0
}

proc updatePoints { x y } {
  echo "updatePoints $x $y"

  set n 100

# $::point_list1 set list.size $n
  $::point_list2 set list.size $n

# set xmin  10000
# set zmin  10000
# set xmax -10000
# set zmax -10000

  for {set i 0} {$i < $n} {incr i} {
  # set z [randIn 0 1]
  # set z [randIn 0.999 0.99999]
  # set z 0.999999
    set z [randIn 0.999 1]

    set p [list $x $y $z]

#   set p1 [sb3d::canvas get point.unproject $p]
    set p2 [sb3d::canvas get point.unview $p]

#   $::point_list1 set point.position $p1 $i
    $::point_list2 set point.position $p2 $i

#   $::point_list1 set point.color red   $i
    $::point_list2 set point.color green $i

#   set x1 [lindex $p2 0]
#   set z1 [lindex $p2 2]

#   if {$x1 < $xmin} { set xmin $x1; set zmin $z }
#   if {$x1 > $xmax} { set xmax $x1; set zmax $z }

    # echo "Z: $z -> [lindex $p2 0]"
  }

# echo "XMin: $xmin, ZMin: $zmin"
# echo "XMax: $xmax, ZMax: $zmax"

  $::line_list set list.size $n

  set z1 0.9
  set p1 [list $x $y $z1]

  set pu1 [sb3d::canvas get point.unview $p1]

  for {set i 0} {$i < $n} {incr i} {
    set i1 [expr {$i + 1}]

#   set z2 [expr {(1.0*$i1)/$n}]
    set z2 [mapReal $i1 0 $n 0.9 1]
    set p2 [list $x $y $z2]

    set pu2 [sb3d::canvas get point.unview $p2]

    $::line_list set line.color yellow $i
    $::line_list set line.start $pu1   $i
    $::line_list set line.end   $pu2   $i

    set z1  $z2
    set p1  $p2
    set pu1 $pu2
  }

  sb3d::canvas exec update
}
