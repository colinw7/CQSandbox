proc init { } {
  set ::graph [sb3d::graph]

  $::graph set dot_file "dot/philo.gv"
}
