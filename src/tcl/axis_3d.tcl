proc init { } {
  set ::xaxis [sb3d::axis]
  set ::yaxis [sb3d::axis]
  set ::zaxis [sb3d::axis]

  $::xaxis set start [list -1.0 0.0 0.0]
  $::xaxis set end   [list  1.0 0.0 0.0]

  $::xaxis set min 1.0
  $::xaxis set max 5.0

  $::yaxis set start [list 0.0 -1.0 0.0]
  $::yaxis set end   [list 0.0  1.0 0.0]

  $::yaxis set min 1.0
  $::yaxis set max 5.0

  $::zaxis set start [list 0.0 0.0 -1.0]
  $::zaxis set end   [list 0.0 0.0  1.0]

  $::zaxis set min 1.0
  $::zaxis set max 5.0
}
