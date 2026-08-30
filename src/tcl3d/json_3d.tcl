proc init { } {
  set ::json [sb3d::json "tcl3d/flare.json"]

  if {! [$::json exec load]} {
    puts "json load failed"
    exit 1
  }

# echo [$::json get to_string]

  echo [$::json get type]
  echo [$::json get num_values]
  echo [$::json get values]

  set prefix ""

  foreach nv [$::json get values] {
    set name [lindex $nv 0]
    set id   [lindex $nv 1]

    echo -nonewline "${name}: "

    printValue $id $prefix
  }

  exit
}

proc printValue { id prefix } {
  set type [$::json get type $id]

  if       {$type == "object"} {
    printObject $id $prefix
  } elseif {$type == "array"} {
    printArray $id $prefix
  } elseif {$type == "string"} {
    echo "${prefix}[$::json get value $id]"
  } elseif {$type == "number"} {
    echo "${prefix}[$::json get value $id]"
  } elseif {$type == "true"} {
    echo "${prefix}true"
  } elseif {$type == "false"} {
    echo "${prefix}false"
  } else {
    echo "${prefix}none"
  }
}

proc printObject { id prefix } {
  echo "${prefix}{"

  foreach nv [$::json get values $id] {
    set name [lindex $nv 0]
    set id1  [lindex $nv 1]

    set type [$::json get type $id1]

    if {$type != "object" && $type != "array"} {
      echo -nonewline "  ${prefix}${name}: "

      echo [$::json get value $id1]
    } else {
      echo "  ${prefix}${name}: "

      printValue $id1 "  $prefix"
    }
  }

  echo "${prefix}}"
}

proc printArray { id prefix } {
  foreach nv [$::json get values $id] {
    set id1 [lindex $nv 1]

    printValue $id1 "  $prefix"
  }
}
