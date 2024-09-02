proc init { } {
  set ::model [sb::csv "csv/AAPL.csv"]

  sb::canvas set range {0 0 100 100}

  $::model set first_line_header 1

  if {! [$::model exec load]} {
    puts "Model load failed"
    exit 1
  }

  set ::nr [$::model get num_rows]
# set ::nc [$::model get num_cols]

  set ::start_row 0
  set ::draw_rows 100

  createObjs

  updateObjs
}

proc createObjs { } {
  set ::group1 [sb::group [list 0 0 100 50]]
  set ::group2 [sb::group [list 0 50 100 100]]

  $::group1 set id group1
  $::group2 set id group2

  set ::points1 [sb::point_list 3px]
  $::points1 set id points1

  $::points1 set size $::nr

  $::points1 set connected 1
  $::points1 set fill_under 1
  $::points1 set fill_under.y 0
  $::points1 set group $::group1
  $::points1 set pen.color green
  $::points1 set brush.color orange

  set ::points2 [sb::point_list 3px]
  $::points2 set id points2

  $::points2 set size $::draw_rows

  $::points2 set connected 1
  $::points2 set fill_under 1
  $::points2 set fill_under.y 0
  $::points2 set group $::group2
  $::points2 set pen.color blue
  $::points2 set brush.color yellow

  set ::wrect [sb::rect [list 0 0 1 1]]
  $::wrect set id wrect

  $::wrect set group $::group1

  $::wrect set brush.color red
  $::wrect set brush.alpha 0.5
}

proc updateObjs { } {
  set min_secs1  0
  set max_secs1  0
  set min_value1 0
  set max_value2 0
  set set1       0

  set min_secs2  0
  set max_secs2  0
  set min_value2 0
  set max_value3 0
  set set2       0

  for {set r 0} {$r < $::nr} {incr r} {
    set date1  [$::model get data $r 0]
    set value1 [$::model get data $r 1]
    set secs1  [clock scan $date1 -format "%m/%d/%Y"]

    if {$set1 == 0} {
      set min_secs1  $secs1
      set max_secs1  $secs1
      set min_value1 $value1
      set max_value1 $value1
    } else {
      set min_secs1  [expr {min($secs1 , $min_secs1 )}]
      set max_secs1  [expr {max($secs1 , $max_secs1 )}]
      set min_value1 [expr {min($value1, $min_value1)}]
      set max_value1 [expr {max($value1, $max_value1)}]
    }

    incr set1

    ###---

    #set r1 [expr {$r + $::start_row}]
    set r1 [expr {$::nr - 1 - $r - $::start_row}]

    if {$r1 >= 0 && $set2 < $::draw_rows} {
      set date2  [$::model get data $r1 0]
      set value2 [$::model get data $r1 1]
      set secs2  [clock scan $date2 -format "%m/%d/%Y"]

      if {$set2 == 0} {
        set min_secs2  $secs2
        set max_secs2  $secs2
        set min_value2 $value2
        set max_value2 $value2
      } else {
        set min_secs2  [expr {min($secs2 , $min_secs2 )}]
        set max_secs2  [expr {max($secs2 , $max_secs2 )}]
        set min_value2 [expr {min($value2, $min_value2)}]
        set max_value2 [expr {max($value2, $max_value2)}]
      }

      incr set2
    }
  }

  #puts "$min_secs1 $min_value1 $max_secs1 $max_value1"
  #puts "$min_secs2 $min_value2 $max_secs2 $max_value2"

  $::group1 set range [list $min_secs1 0 $max_secs1 $max_value1]
  $::group2 set range [list $min_secs2 0 $max_secs2 $max_value2]
# $::group2 set range [list $min_secs2 0 $max_secs2 $max_value1]

  $::wrect set rect [list $min_secs2 0 $max_secs2 $max_value1]

  set last_secs1  0
  set last_value1 0
  set last_secs2  0
  set last_value2 0

  set set1 0
  set set2 0

  for {set r 0} {$r < $::nr} {incr r} {
    set date1  [$::model get data $r 0]
    set value1 [$::model get data $r 1]
    set secs1  [clock scan $date1 -format "%m/%d/%Y"]

    $::points1 set position [list $secs1 $value1] $set1

    set last_secs1  $secs1
    set last_value1 $value1

    incr set1

    ###---

    #set r1 [expr {$r + $::start_row}]
    set r1 [expr {$::nr - 1 - $r - $::start_row}]

    if {$r1 >= 0 && $set2 < $::draw_rows} {
      set date2  [$::model get data $r1 0]
      set value2 [$::model get data $r1 1]
      set secs2  [clock scan $date2 -format "%m/%d/%Y"]

      $::points2 set position [list $secs2 $value2] $set2

      set last_secs2  $secs2
      set last_value2 $value2

      incr set2
    }
  }
}
  
proc update { } {
  incr ::start_row

  updateObjs
}
