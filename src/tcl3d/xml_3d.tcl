proc printTagNames { {id ""} {prefix ""} } {
  echo "${prefix}Tag [$::xml get name $id]:$id"

  set prefix1 "  $prefix"

  if {$id != ""} {
    set tags [$::xml get tag_names $id]
  } else {
    set tags [$::xml get tag_names]
  }
  
  if {[llength $tags] > 0} {
    echo -nonewline "${prefix1}Tags:"
    foreach tag $tags {
      echo -nonewline " $tag"
    }
    echo ""
  }

  if {$id != "root"} {
    if {$id != ""} {
      set options [$::xml get tag_options $id]
    } else {
      set options [$::xml get tag_options]
    }

    if {[llength $options] > 0} {
      echo -nonewline "${prefix1}Options:"
      foreach option $options {
        set name  [lindex $option 0]
        set value [lindex $option 1]

        echo -nonewline " $name=$value"
      }
      echo ""
    }
  }

  if {$id != ""} {
    set children [$::xml get tag_inds $id]
  } else {
    set children [$::xml get tag_inds]
  }

  if {[llength $children] > 0} {
    foreach child $children {
      printTagNames $child $prefix1
    }
  }
}

proc init { } {
  #set ::xml [sb3d::xml "tcl3d/maze.xml"]
  set ::xml [sb3d::xml "tcl3d/dungeon.xml"]

  if {! [$::xml exec load]} {
    puts "Xml load failed"
    exit 1
  }

  printTagNames root

  printTagNames

  exit
}
