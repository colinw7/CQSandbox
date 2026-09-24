proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min)}]
}

proc init { } {
  set ::TWO_PI [expr {2*$::PI}]

  set ::penx 0
  set ::peny 0

  set ::de 2

  set ::lista [$::ClistA create]

  set ::el0 0

  set ::by 0
}

# --

proc setup { } {
  set ::renderer [sb::renderer]

  $::renderer set size {800 600}
# size(1024, 768);
# size(screen.width, screen.height);

  # smooth()
  # frameRate(30);
  # colorMode(HSB, 1);
  # ellipseMode(CENTER);

  $::renderer set pen.color [list hsb 0 0 0 0.7]

  $::renderer set brush.color [list hsb 0 0 1]

# $::renderer set pen.width [expr {0.3*$::de + 1}]
  $::renderer set pen.width 2

  city
}

# --

proc draw { } {
  set t 0
  while {1} {
    incr t
    if {$t >= 50 || ($::el0 < [$::lista get nr])} {
      break
    }
    $::lista exec draw $::el0
    incr ::el0
  }

  if {$::el0 == [$::lista get nr]} {
    sb::canvas set play 0
  }
}

# --

proc mouseClicked { } {
  $::lista set nr 0
  set ::el0 0
  city
  $::renderer set brush.color [list hsb 0 0 1]
  sb::canvas set play 1
}

# --

proc keyPressed { } {
  if {$key == "s"} {
    save("city.tif");
  }
}

# --

proc city { } {
  # cielo nero
# rettf 0 0 $width [expr {int(0.95*$height)}]

  # luna
  set rag [expr {$height/10}]

  $::lista exec agg 0 [irandIn $rag [expr {$width-$rag}]] \
                      [irandIn [expr {0.3*$height}] [expr {0.5*$height-$rag}]]
  $::lista exec agg 6 $rag 0

  # strade
  set p1 [proietta [$::PVector create -10 0 240]]
  set p2 [proietta [$::PVector create -10 0 -60]]
  linea $p1 $p2

  set p1 [proietta [$::PVector create -20 0 240]]
  set p2 [proietta [$::PVector create -20 0 -60]]
  linea $p1 $p2

  set p1 [proietta [$::PVector create -40 0  -5]]
  set p2 [proietta [$::PVector create 240 0  -5]]
  linea $p1 $p2

  set p1 [proietta [$::PVector create -40 0 -15]]
  set p2 [proietta [$::PVector create 240 0 -15]]
  linea $p1 $p2

  palazzo [$::PVector create 40 0 80]
  palazzo [$::PVector create 40 0 40]

  for {set z 9} {$z > 0} {incr z -1} {
    palazzo [$::PVector create 0 0 [expr {$z*40}]]
  }

  for {set x 6} {$x >= 0} {incr x -1} {
    palazzo [$::PVector create [expr {$x*40}] 0 0]
  }

  # alberi
  for {set x 220} {$x >= 0} {incr x -40} {
    albero [$::PVector create $x 0 -2]
    albero [$::PVector create -2 0 $x]
  }

  # semafori
  semaforox [$::PVector create  0 0 162]
  semaforox [$::PVector create  0 0  82]
  semaforox [$::PVector create  0 0   2]
  semaforoz [$::PVector create 82 0   0]
  semaforoz [$::PVector create  2 0   0]

  palazzo [$::PVector create 0 0 -53]
}

# --

proc proietta { pt } {
  set d 60.00
  set h 1.50

  set alfa [radians 30]

  set a [expr {[$pt get x]*sin($alfa) - [$pt get z]*cos($alfa)}]
  set b [expr {[$pt get x]*cos($alfa) + [$pt get z]*sin($alfa)}]
  set x [expr {$d * $a / ($b + $d)}]
  set y [expr {$d * ([$pt get y] - $h) / ($b + $d)}]

  set sc [expr {$height/90}]

  return [$::PVector create [expr {0.63*$width + $sc*$x}] [expr {0.95*$height - $sc*$y}]]
}

# --

proc palazzo { pt } {
  set tipo [irandIn 0 10]
  set h    [irandIn 50 100]

  set insegne 0

  if       {$tipo == 0} {
    box3d [$::PVector create [expr {[$pt get x] + 20}] $h [expr {[$pt get z] + 10}]]
          [$::PVector create [expr {[$pt get x] + 21}] [expr {$h + [randIn 20 40]}]
                             [expr {[$pt get z] + 11}]]

    box3d [$::PVector create [expr {[$pt get x] + 10}] $h [expr {[$pt get z] + 10}]]
          [$::PVector create [expr {[$pt get x] + 11}] [expr {$h + [randIn 20 40]}]
                             [expr {[$pt get z] + 11}]]

    rettangolo_bianco [$::PVector create [expr {[$pt get x]     }] $h [expr {[$pt get z]     }]]
                      [$::PVector create [expr {[$pt get x] + 30}] $h [expr {[$pt get z]     }]]
                      [$::PVector create [expr {[$pt get x] + 30}] $h [expr {[$pt get z] + 30}]]
                      [$::PVector create [expr {[$pt get x]     }] $h [expr {[$pt get z] + 30}]]

    box3d [$::PVector create [expr {[$pt get x]     }]              20 [expr {[$pt get z]     }]]
          [$::PVector create [expr {[$pt get x] + 30}] [expr {$h - 4}] [expr {[$pt get z] + 30}]]

    for {set fi [expr {30-3}]} {$fi > 0} {incr fi -3} {
      linea [proietta [$::PVector create [expr {[$pt get x]      }] 20
                                         [expr {[$pt get z] + $fi}]
            [proietta [$::PVector create [expr {[$pt get x]      }] [expr {$h - 4}]
                                         [expr {[$pt get z] + $fi}]
      linea [proietta [$::PVector create [expr {[$pt get x] + $fi}] 20
                                         [expr {[$pt get z]}]
            [proietta [$::PVector create [expr {[$pt get x] + $fi}] [expr {$h - 4}]
                                         [expr {[$pt get z]}]
    }

    box3d $pt [$::PVector create [expr {[$pt get x] + 30}] 19 [expr {[$pt get z] + 30}]]

    set insegne 1
  } elseif {$tipo == 1} {
    set nr [irandIn 5 20]

    for {set i [expr {$nr - 1}]} {$i >= 0} {incr i -1} {
      box3d [$::PVector create [$pt get x], i*4+4, [$pt get z]),
            [$::PVector create [$pt get x]+30, i*4+6, [$pt get z]+30))
    }

    pilotis $pt 30 30 4

    set insegne 1
  } elseif {$tipo == 2} {
    box3d [$::PVector create [$pt get x]+1, h-10, [$pt get z]+1),
          [$::PVector create [$pt get x]+29, h, [$pt get z]+29));
    box3d [$::PVector create [$pt get x]+ 2, 10, [$pt get z]+22),
          [$::PVector create [$pt get x]+ 4, h-10, [$pt get z]+28));
    box3d [$::PVector create [$pt get x]+22, 10, [$pt get z]+ 2),
          [$::PVector create [$pt get x]+28, h-10, [$pt get z]+ 4));
    box3d [$::PVector create [$pt get x]+ 4, 10, [$pt get z]+ 4),
          [$::PVector create [$pt get x]+22, h-10, [$pt get z]+22));
    box3d [$::PVector create [$pt get x]+ 2, 10, [$pt get z]+ 2),
          [$::PVector create [$pt get x]+ 8, h-10, [$pt get z]+ 8));

    basamento $pt 10

    set insegne 1

  } elseif {$tipo == 3}  {
    box3d [$::PVector create [$pt get x] + 14.5, h, [$pt get z]+14.5),
          [$::PVector create [$pt get x] + 15.5, h+40, [$pt get z]+15.5));

    if {$h > 50} {
      box3d [$::PVector create([$pt get x]+2, 50, [$pt get z]+2),
            [$::PVector create [$pt get x]+28, h, [$pt get z]+28));
    }
    if {$h > 50} {
      box3d [$::PVector create([$pt get x]+4, 42, [$pt get z]+4),
            [$::PVector create [$pt get x]+26, 50, [$pt get z]+26));
    }
    if {$h > 42} {
      box3d [$::PVector create([$pt get x]+2, 16, [$pt get z]+2),
            [$::PVector create [$pt get x]+28, 42, [$pt get z]+28));
    }
    if {$h > 16} {
      box3d [$::PVector create([$pt get x]+4, 12, [$pt get z]+4),
            [$::PVector create [$pt get x]+26, 16, [$pt get z]+26));
    }

    if {h > 12} {
      basamento $pt 12
    }

    set insegne 1
  } elseif {$tipo == 4} {
    cilindro3d [$::PVector create [$pt get x], 31, [$pt get z]), h-16, 32, 13, true);
    cilindro3d [$::PVector create [$pt get x], 26, [$pt get z]),    4, 32, 13, true);
    cilindro3d [$::PVector create [$pt get x], 21, [$pt get z]),    4, 32, 13, true);
    cilindro3d [$::PVector create [$pt get x], 16, [$pt get z]),    4, 32, 13, true);
    cilindro3d [$::PVector create [$pt get x], 12, [$pt get z]),    4, 32, 11, true);
    cilindro3d [$::PVector create [$pt get x],  0, [$pt get z]),   12, 32, 15, false);
  } elseif {$tipo == 5} {
    cilindro3d [$::PVector create [$pt get x], h-2, [$pt get z]), 2, 8, 13, true);
    cilindro3d [$::PVector create [$pt get x], h-6, [$pt get z]), 4, 8, 9, true);
    cilindro3d [$::PVector create [$pt get x], 8, [$pt get z]), h-14, 8, 13, true);

    basamento $pt 8

    set insegne 1
  } elseif {$tipo == 6: # giardino
    box3d $pt, [$::PVector create [$pt get x]+30, [$pt get y]+1, [$pt get z]+30));

    for {set i 0} {$i < 10} {incr i} {
      set pz [map $i 0 10 0 22]
      set px [randIn 0 $pz]

      set pt0 [$::PVector create [$pt get x]+26-px, [$pt get y]+1, [$pt get z]+26-pz+px);

      albero $pt0
    }
  } elseif {$tipo == 7} {
    box3d [$::PVector create [$pt get x] +  5, 32, [$pt get z] +  5]
          [$::PVector create [$pt get x] + 25, $h, [$pt get z] + 25]
    box3d [$::PVector create [$pt get x] + 10, 26, [$pt get z] + 10]
          [$::PVector create [$pt get x] + 20, 32, [$pt get z] + 20]
    box3d [$::PVector create [$pt get x] +  5, 26, [$pt get z] + 24]
          [$::PVector create [$pt get x] +  5, 32, [$pt get z] + 25]
    box3d [$::PVector create [$pt get x] +  5, 26, [$pt get z] + 18]
          [$::PVector create [$pt get x] +  5, 32, [$pt get z] + 19]
    box3d [$::PVector create [$pt get x] +  5, 26, [$pt get z] + 11]
          [$::PVector create [$pt get x] +  5, 32, [$pt get z] + 12]
    box3d [$::PVector create [$pt get x] + 24, 26, [$pt get z] +  5]
          [$::PVector create [$pt get x] + 25, 32, [$pt get z] +  6]
    box3d [$::PVector create [$pt get x] + 18, 26, [$pt get z] +  5]
          [$::PVector create [$pt get x] + 19, 32, [$pt get z] +  6]
    box3d [$::PVector create [$pt get x] + 11, 26, [$pt get z] +  5]
          [$::PVector create [$pt get x] + 12, 32, [$pt get z] +  6]
    box3d [$::PVector create [$pt get x] +  5, 26, [$pt get z] +  5]
          [$::PVector create [$pt get x] +  6, 32, [$pt get z] +  6]

    cilindro3d [$::PVector create [$pt get x] 6 [$pt get z]) 20 25 15 0

    box3d [$::PVector create [$pt get x]+10, 0, [$pt get z]+10),
          [$::PVector create [$pt get x]+20, 6, [$pt get z]+20));
    box3d [$::PVector create [$pt get x]+5, 0, [$pt get z]+24),
          [$::PVector create [$pt get x]+5, 6, [$pt get z]+25));
    box3d [$::PVector create [$pt get x]+5, 0, [$pt get z]+18),
          [$::PVector create [$pt get x]+5, 6, [$pt get z]+19));
    box3d [$::PVector create [$pt get x]+5, 0, [$pt get z]+11),
          [$::PVector create [$pt get x]+5, 6, [$pt get z]+12));
    box3d [$::PVector create [$pt get x]+24, 0, [$pt get z]+5),
          [$::PVector create [$pt get x]+25, 6, [$pt get z]+6));
    box3d [$::PVector create [$pt get x]+18, 0, [$pt get z]+5),
          [$::PVector create [$pt get x]+19, 6, [$pt get z]+6));
    box3d [$::PVector create [$pt get x]+11, 0, [$pt get z]+5),
          [$::PVector create [$pt get x]+12, 6, [$pt get z]+6));
    box3d [$::PVector create [$pt get x]+ 5, 0, [$pt get z]+5),
          [$::PVector create [$pt get x]+ 6, 6, [$pt get z]+6));

    set insegne 1
  } elseif {$tipo == 8} {
    box3d [$::PVector create [$pt get x] +  2,       6, [$pt get z] +  2]
          [$::PVector create [$pt get x] + 18, $h - 10, [$pt get z] + 18]
    box3d [$::PVector create [$pt get x]     ,       0, [$pt get z] + 23]
          [$::PVector create [$pt get x] +  2, $h     , [$pt get z] + 28]
    box3d [$::PVector create [$pt get x]     ,       0, [$pt get z] + 18]
          [$::PVector create [$pt get x] +  2, $h -  5, [$pt get z] + 23]
    box3d [$::PVector create [$pt get x]     ,       0, [$pt get z] +  7]
          [$::PVector create [$pt get x] +  2, $h -  5, [$pt get z] + 12]
    box3d [$::PVector create [$pt get x]     ,       0, [$pt get z] +  2]
          [$::PVector create [$pt get x] +  2, $h     , [$pt get z] +  7]
    box3d [$::PVector create [$pt get x] + 23,       0, [$pt get z]     ]
          [$::PVector create [$pt get x] + 28, $h     , [$pt get z] +  2]
    box3d [$::PVector create [$pt get x] + 18,       0, [$pt get z]     ]
          [$::PVector create [$pt get x] + 23, $h -  5, [$pt get z] +  2]
    box3d [$::PVector create [$pt get x] +  7,       0, [$pt get z]     ]
          [$::PVector create [$pt get x] + 12, $h -  5, [$pt get z] +  2]
    box3d [$::PVector create [$pt get x] +  2,       0, [$pt get z]     ]
          [$::PVector create [$pt get x] +  7, $h     , [$pt get z] +  2]

    set insegne 1
  } elseif {$tipo == 9} {
    cilindro3d [$::PVector create [$pt get x] 0 [$pt get z]] $h 32 11 1

    box3d [$::PVector create [expr {[$pt get x]     }]  0 [expr {[$pt get z] + 15}]]
          [$::PVector create [expr {[$pt get x] + 4 }] $h [expr {[$pt get z] + 30}]]
    box3d [$::PVector create [expr {[$pt get x] + 15}]  0 [expr {[$pt get z]     }][
          [$::PVector create [expr {[$pt get x] + 30}] $h [expr {[$pt get z] +  4}]]

    set insegne 1
  }

  if {$insegne} {
    set iw   [randIn 1 1.5]
    set ih   [randIn 2 10]
    set ipos [randIn 2 14]

    rettangolo_bianco [$::PVector create [$pt get x]+ipos, 4, [$pt get z]-1),
                      [$::PVector create [$pt get x]+ipos, 4+ih, [$pt get z]-1),
                      [$::PVector create [$pt get x]+ipos, 4+ih, [$pt get z]-1-iw),
                      [$::PVector create [$pt get x]+ipos, 4, [$pt get z]-1-iw));

    set iw   [randIn 1 1.5]
    set ih   [randIn 2 10]
    set ipos [expr {[randIn 2 14] + 15}]

    rettangolo_bianco [$::PVector create [$pt get x]+ipos, 4, [$pt get z]-1),
                      [$::PVector create [$pt get x]+ipos, 4+ih, [$pt get z]-1),
                      [$::PVector create [$pt get x]+ipos, 4+ih, [$pt get z]-1-iw),
                      [$::PVector create [$pt get x]+ipos, 4, [$pt get z]-1-iw));

    set iw   [randIn (1, 1.5]
    set ih   [randIn (2, 10]
    set ipos [randIn (2, 14]

    rettangolo_bianco [$::PVector create [$pt get x]-1, 4, [$pt get z]+ipos),
                      [$::PVector create [$pt get x]-1, 4+ih, [$pt get z]+ipos),
                      [$::PVector create [$pt get x]-1-iw, 4+ih, [$pt get z]+ipos),
                      [$::PVector create [$pt get x]-1-iw, 4, [$pt get z]+ipos));

    set iw   [randIn 1 1.5]
    set ih   [randIn 2 10]
    set ipos [expr {[randIn 2 14] + 15}]

    rettangolo_bianco [$::PVector create [expr {[$pt get x] - 1      }]
                      [expr {4,     }] [expr {[$pt get z] + $ipos}]]
                      [$::PVector create [expr {[$pt get x] - 1      }]
                      [expr {4 + $ih}] [expr {[$pt get z] + $ipos}]]
                      [$::PVector create [expr {[$pt get x] - 1 - $iw}]
                      [expr {4 + $ih}] [expr {[$pt get z] + $ipos}]]
                      [$::PVector create [expr {[$pt get x] - 1 - $iw}]
                      [expr {4      }] [expr {[$pt get z] + $ipos}]]
  }
}

# --

proc basamento { pt h } {
  set nr1 [irandIn 1 5]
  set nr2 [irandIn 1 5]

  set inter1 [expr {27.0/$nr1}
  set inter2 [expr {27.0/$nr2}

  box3d $pt [$::PVector create([$pt get x]+30, h, [$pt get z]+30));

  rettangolo_rigato [$::PVector create([$pt get x], 6, [$pt get z]+2),
                    [$::PVector create([$pt get x], h-2, [$pt get z]+2),
                    [$::PVector create([$pt get x], h-2, [$pt get z]+28),
                    [$::PVector create([$pt get x], 6, [$pt get z]+28));

  for {set i 0} {$i < $nr1} {incr i} {
    rettangolo_rigato [$::PVector create([$pt get x], 0, [$pt get z]+2+i*inter1),
                      [$::PVector create([$pt get x], 4, [$pt get z]+2+i*inter1),
                      [$::PVector create([$pt get x], 4, [$pt get z]+1+(i+1)*inter1),
                      [$::PVector create([$pt get x], 0, [$pt get z]+1+(i+1)*inter1));
  }
  rettangolo_nero [$::PVector create([$pt get x]+2, 6, [$pt get z]),
                  [$::PVector create([$pt get x]+2, h-2, [$pt get z]),
                  [$::PVector create([$pt get x]+28, h-2, [$pt get z]),
                  [$::PVector create([$pt get x]+28, 6, [$pt get z]));

  for {set i 0} {$i < $nr2} {incr i} {
    rettangolo_nero [$::PVector create([$pt get x]+2+i*inter2, 0, [$pt get z]),
                    [$::PVector create([$pt get x]+2+i*inter2, 4, [$pt get z]),
                    [$::PVector create([$pt get x]+1+(i+1)*inter2, 4, [$pt get z]),
                    [$::PVector create([$pt get x]+1+(i+1)*inter2, 0, [$pt get z]));
  }
}

# --

proc albero { pt0 } {
  PVector pt1 = pt0.get();
  [$pt1 get y] += 4;

  set pt2 $pt1

  [$pt2 get x] += 2;
  [$pt2 get z] -= 2;

  set pp0 [proietta $pt0]
  set pp1 [proietta $pt1]
  set pp2 [proietta $pt2]

  $::lista exec agg 0 [expr {[$pp1 get x]                }] [expr {(int)pp1.y}]
  $::lista exec agg 6 [expr {[$pp2 get x] - [$pp1 get x])}] 0

  dlinea $pp0 $pp1
}

# --

proc semaforox { pt0 } {
  linea [proietta [$::PVector create [$pt0 get x]  -2 0 [$pt0 get z]]]
        [proietta [$::PVector create [$pt0 get x]  -2 5 [$pt0 get z]]]
  linea [proietta [$::PVector create [$pt0 get x] -28 0 [$pt0 get z]]]
        [proietta [$::PVector create [$pt0 get x] -28 5 [$pt0 get z]]]
  linea [proietta [$::PVector create [$pt0 get x]  -2 4 [$pt0 get z]]]
        [proietta [$::PVector create [$pt0 get x] -28 4 [$pt0 get z]]]
  linea [proietta [$::PVector create [$pt0 get x]  -2 5 [$pt0 get z]]]
        [proietta [$::PVector create [$pt0 get x] -28 5 [$pt0 get z]]]

  rettangolo_bianco [$::PVector create [$pt0 get x]  -4 4 [$pt0 get z]]
                    [$::PVector create [$pt0 get x]  -4 5 [$pt0 get z]]
                    [$::PVector create [$pt0 get x]  -9 5 [$pt0 get z]]
                    [$::PVector create [$pt0 get x]  -9 4 [$pt0 get z]]
  rettangolo_bianco [$::PVector create [$pt0 get x] -11 4 [$pt0 get z]]
                    [$::PVector create [$pt0 get x] -11 5 [$pt0 get z]]
                    [$::PVector create [$pt0 get x] -19 5 [$pt0 get z]]
                    [$::PVector create [$pt0 get x] -19 4 [$pt0 get z]]
  rettangolo_bianco [$::PVector create [$pt0 get x] -21 4 [$pt0 get z]]
                    [$::PVector create [$pt0 get x] -21 5 [$pt0 get z]]
                    [$::PVector create [$pt0 get x] -26 5 [$pt0 get z]]
                    [$::PVector create [$pt0 get x] -26 4 [$pt0 get z]]
}

# --

proc semaforoz { pt0 } {
  linea [proietta [$::PVector create [$pt0 get x] 0 [expr {[$pt0 get z] -2 }]]]
        [proietta [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z] -2 }]]]
  linea [proietta [$::PVector create [$pt0 get x] 0 [expr {[$pt0 get z] -18}]]]
        [proietta [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z] -18}]]]
  linea [proietta [$::PVector create [$pt0 get x] 4 [expr {[$pt0 get z]  -2}]]]
        [proietta [$::PVector create [$pt0 get x] 4 [expr {[$pt0 get z] -18}]]]
  linea [proietta [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z]  -2}]]]
        [proietta [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z] -18}]]]

  rettangolo_bianco [$::PVector create [$pt0 get x] 4 [expr {[$pt0 get z] -4}]]
                    [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z] -4}]]
                    [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z] -9}]]
                    [$::PVector create [$pt0 get x] 4 [expr {[$pt0 get z] -9}]]
  rettangolo_bianco [$::PVector create [$pt0 get x] 4 [expr {[$pt0 get z]-11}]]
                    [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z]-11}]]
                    [$::PVector create [$pt0 get x] 5 [expr {[$pt0 get z]-16}]]
                    [$::PVector create [$pt0 get x] 4 [expr {[$pt0 get z]-16}]]
}

# --

proc box3d { pt1 pt2 } {
  set p1 [proietta [$::PVector create [$pt1 get x] [$pt1 get y] [$pt2 get z]]]
  set p2 [proietta [$::PVector create [$pt1 get x] [$pt2 get y] [$pt2 get z]]]
  set p3 [proietta [$::PVector create [$pt1 get x] [$pt1 get y] [$pt1 get z]]]
  set p4 [proietta [$::PVector create [$pt1 get x] [$pt2 get y] [$pt1 get z]]]
  set p5 [proietta [$::PVector create [$pt2 get x] [$pt1 get y] [$pt1 get z]]]
  set p6 [proietta [$::PVector create [$pt2 get x] [$pt2 get y] [$pt1 get z]]]
  set p7 [proietta [$::PVector create [$pt2 get x] [$pt1 get y] [$pt2 get z]]]

  $::lista exec agg 3 [$p1 get x] [$p1 get y]
  $::lista exec agg 3 [$p2 get x] [$p2 get y]
  $::lista exec agg 3 [$p4 get x] [$p4 get y]
  $::lista exec agg 3 [$p6 get x] [$p6 get y]
  $::lista exec agg 3 [$p5 get x] [$p5 get y]
  $::lista exec agg 4 [$p7 get x] [$p7 get y]

  if {[$p7 get y] > [$p1 get y]} {
    $::lista exec agg 3 [$p1 get x [$p1 get y
    $::lista exec agg 3 [$p3 get x [$p3 get y
    $::lista exec agg 3 [$p5 get x [$p5 get y
    $::lista exec agg 5 [$p7 get x [$p7 get y
  }

  dlinea [$p1 get x], [$p1 get y] [$p3 get x] [$p3 get y]
  dlinea [$p1 get x], [$p1 get y] [$p2 get x] [$p2 get y]
  dlinea [$p2 get x], [$p2 get y] [$p4 get x] [$p4 get y]
  dlinea [$p3 get x], [$p3 get y] [$p4 get x] [$p4 get y]
  dlinea [$p3 get x], [$p3 get y] [$p5 get x] [$p5 get y]
  dlinea [$p5 get x], [$p5 get y] [$p6 get x] [$p6 get y]
  dlinea [$p4 get x], [$p4 get y] [$p6 get x] [$p6 get y]

  if {p7.y > p1.y} {
    dlinea [$p1 get x] [$p1 get y] [$p7 get x] [$p7 get y]
    dlinea [$p5 get x] [$p5 get y] [$p7 get x] [$p7 get y]
  }

  striscia pt2.y - [$pt1 get y], p3, p4, p5, p6);
}

# --

proc cilindro3d { pt h nrseg rad contorno } {
  PVector [][] pv = [$::PVector create[nrseg][2];
  for {set i 0} {$i < $nrseg} {incr i} {
    set alfa [map $i 0 $nrseg 0 $::TWO_PI]
    pv[i][0] = proietta([$::PVector create([$pt get x]+15+rad*cos(alfa), [$pt get y], [$pt get z]+15-rad*sin(alfa)));
    pv[i][1] = proietta([$::PVector create([$pt get x]+15+rad*cos(alfa), [$pt get y]+h, [$pt get z]+15-rad*sin(alfa)));
  }
  for {set i 0} {$i < $nrseg} {incr i} {
    set j [expr {($i + $nrseg - 1) % $nrseg]

    if {[$pv[i][0] get x] <= [$pv[j][0] get x]} {
      if {$i > 0} {
        $::lista exec agg 3 [$pv[j][0] get x] [$pv[j][0] get y]
        $::lista exec agg 3 [$pv[i][0] get x] [$pv[i][0] get y]
        $::lista exec agg 3 [$pv[i][1] get x] [$pv[i][1] get y]
        $::lista exec agg 4 [$pv[j][1] get x] [$pv[j][1] get y]
      }
    }
  }
  for {set i 0} {$i < $nrseg} {incr i} {
    $::lista exec agg (3, (int)pv[i][0].x, (int)pv[i][0].y);
  }
  $::lista exec agg (5, (int)pv[0][0].x, (int)pv[0][0].y);
  for {set i 0} {$i < $nrseg} {incr i} {
    int j = (i + nrseg - 1) % nrseg;
    set alfa [expr {[map $i 0 nrseg 0 $::TWO_PI] + 1}]
    if {[$pv[$i][0] get x] <= [$pv[$j][0] get x]} {
      if {[$pv[($i+1)%$nrseg][0] get x] >= [$pv[i][0] get x]} {
        linea(pv[i][0], pv[i][1]);
      }
      if {$contorno} {
        linea $pv[$i][0] $pv[$i][1]
      }
      if {$i > 0} {
        dlinea $pv[$j][0] $pv[$i][0]
        dlinea $pv[$j][1] $pv[$i][1]

        if {sin($alfa) > 0} {
          striscia $h $pv[i][0] $pv[i][1] $pv[j][0] $pv[j][1]
        }
      }
    }
    else {
      if {[$pv[($i+1)%@nrseg][0] get x] <= [$pv[$i][0] get x]} {
        linea $pv[$i][0] $pv[$i][1]
      }
    }
  }
}

# --

proc pilotis { pt0 dex dez h } {
  set nrx [expr {int(($dex - 1)/6)}]
  set nrz [expr {int(($dez - 1)/6)}]

  set delx [expr {($dex - 1)/$nrx}]
  set delz [expr {($dez - 1)/$nrz}]

  for {set i $nrz} {$i > 0} {incr i -1} {
    box3d [$::PVector create(pt0.x, 0, pt0.z+i*delz),
          [$::PVector create(pt0.x+1, h, pt0.z+i*delz+1));
  }

  for {set i $nrx} {$i >= 0} {incr i -1} {
    box3d [$::PVector create(pt0.x+i*delx, 0, pt0.z),
          [$::PVector create(pt0.x+i*delx+1, h, pt0.z+1));
  }
}

# --

# striscia tratteggiata in orizzontale
proc striscia { del p1 p2 p3 p4 } {
  set r1 [expr {[$p1 get y - [$p2 get y]/$del}]
  set r2 [expr {[$p3 get y - [$p4 get y]/$del}]

  set pp 0

  while {$pp < $del} {
    linea int(p1.x), int(p2.y + pp * r1), int(p3.x), int(p4.y + pp * r2));

    pp += 0.5;
  }
}

# --

proc rettangolo_bianco { pt1 pt2 pt3 pt4 } {
  set p1 [proietta $pt1]
  set p2 [proietta $pt2]
  set p3 [proietta $pt3]
  set p4 [proietta $pt4]

  $::lista exec agg (3, (int)p1.x, (int)p1.y);
  $::lista exec agg (3, (int)p2.x, (int)p2.y);
  $::lista exec agg (3, (int)p3.x, (int)p3.y);
  $::lista exec agg (4, (int)p4.x, (int)p4.y);

  linea $p1 $p2
  linea $p2 $p3
  linea $p3 $p4
  linea $p4 $p1
}

proc rettangolo_rigato { pt1 pt2 pt3 pt4 } {
  rettangolo_bianco $pt1 $pt2 $pt3 $pt4

  striscia [expr {[$pt2 get y] - [$pt1 get y]}]
           [proietta $pt1] [proietta $pt2] [proietta $pt4] [proietta $pt3]
}

proc rettangolo_nero { pt1 pt2 pt3 pt4 } {
  set p1 [proietta $pt1]
  set p2 [proietta $pt2]
  set p3 [proietta $pt3]
  set p4 [proietta $pt4]

  $::lista exec agg 3 [$p1 get x] [$p1 get y]
  $::lista exec agg 3 [$p2 get x] [$p2 get y]
  $::lista exec agg 3 [$p3 get x] [$p3 get y]
  $::lista exec agg 5 [$p4 get x] [$p4 get y]
}

# --

# rettangolo pieno
proc rettf { sx sy ex ey } {
  set dex [expr {$ex - $sx}]
  set dey [expr {$ey - $sy}]

  $::lista exec agg (0, sx, sy);

  for {set i 0} {$i < min($dex, $dey)} {incr i 6} {
    $::lista exec agg (1, sx, sy + i);
    $::lista exec agg (1, sx + i, sy);
  }

  if {$dex > $dey} {
    for {set i 0} {$i < $dex - $dey} {incr i 6} {
      $::lista exec agg (1, sx + i, ey);
      $::lista exec agg (1, sx + dey + i, sy);
    }
  }
  else {
    for {set i 0} {$i < $dey - $dex} {incr i 6} {
      $::lista exec agg (1, sx, sy + dex + i);
      $::lista exec agg (1, ex, sy + i);
    }
  }

  for {set i [expr {min($dex, $dey)}]} {$i >= 0} {incr i -6} {
    $::lista exec agg 1 [expr {$ex - $i}] $ey
    $::lista exec agg 1 $ex            }] [expr {$ey - $i}]
  }
}

# --

proc linea { sx sy ex ey } {
  $::lista exec agg 0 $sx $sy
  $::lista exec agg 1 $ex $ey
}

proc linea { p1 p2 } {
  linea [$p1 get x] [$p1 get y] [$p2 get x] [$p2 get y]
}

# --

proc dlinea { sx sy ex ey } {
  linea $sx $sy $ex $ey
  linea $sx $sy $ex $ey
}

proc dlinea { p1 p2 } {
  dlinea [$p1 get x] [$p1 get y] [$p2 get x] [$p2 get y]
}

# ---

set ::PVector [sb::class]

$::PVector proc init { v x y } {
  $v set x $x
  $v set y $y
}

# ---

set ::ClistA [sb::class ClistA]

$::CListA proc init { l } {
  set mat [sb::matrix 100000 3]

  $l set mat $mat
  $l set nr  0
  $l set nrv 0

  set pv [sb::array 100]

  for {set i 0} {$i < [$pv get dim]} {incr i} {
    $pv set value $i [$::PVector create 0 0]
  }

  $l set pv $pv
}

$::CListA proc agg { l tipo px py } {
  set mat[$nr][0] $tipo;
  set mat[$nr][1] [expr {$px + [irandIn -$::de  $::de]}]
  set mat[$nr][2] [expr {$py + [irandIn -$::de  $::de]}]

  incr nr
}

$::CListA proc _draw { l el } {
  set mat [$l get mat]

  set m [$mat get $el 0]

  if       {$m == 0} {
    # move to
    set ::penx $mat[el][1];
    set ::peny $mat[el][2];
  } elseif {$m == 1} {
    # line to
    stroke(0, 0, 0, 0.5);
    noFill();
    line($::penx, $::peny, mat[el][1], mat[el][2]);
    set ::penx $mat[el][1];
    set ::peny $mat[el][2];
  } elseif {$m 2} {
    # rettangolo pieno
    noStroke();
    fill(0, 0, 1);
    rect($::penx, $::peny, mat[el][1] - $::penx, mat[el][2] - $::peny);
  } elseif {$m == 3} {
    # vertice poligono pieno
    pv[nrv].x = mat[el][1];
    pv[nrv].y = mat[el][2];
    nrv++;
  } elseif {$m == 4} {
    # chiusura poligono pieno bianco
    pv[nrv].x = mat[el][1];
    pv[nrv].y = mat[el][2];
    nrv++;
    noStroke();
    fill(0, 0, 1);
    beginShape();
    for {set i 0} {$i < $nrv} {incr i} {
      vertex(pv[i].x, pv[i].y);
    }
    endShape(CLOSE);
    nrv = 0;
  } elseif {$m == 5} {
    # chiusura poligono pieno nero
    pv[nrv].x = mat[el][1];
    pv[nrv].y = mat[el][2];
    nrv++;
    noStroke();
    fill(0, 0, 0);
    beginShape();
    for {set i 0} {$i < $nrv} {incr i} {
      vertex(pv[i].x, pv[i].y);
    }
    endShape(CLOSE);
    nrv = 0;
  } elseif {$m == 6} {
    # sfera
    set px1 = 0;
    set py1 = 0;
    set nrs [expr {$height/6}]
    set ra [randIn 2.1 2.4]
    noStroke();
    fill(0, 0, 1);
    ellipse($::penx, $::peny, 2*mat[el][1], 2*mat[el][1]);
    stroke(0, 0, 0, 0.5);
    noFill();
    for {set i 0} {$i < $nrs} {incr i} {
      set alfa [map $i 0 $nrs 0 2*$::TWO_PI]
      set px0 $px1
      set py0 $py1
      set rad [expr {$mat[el][1] + 2*$::de*sin(ra*alfa)}]
      set px1 [expr {$::penx + int(rad * cos(alfa))}]
      set py1 [expr {$::peny + int(rad * sin(alfa))}]
      if {$i > 0} {
        line(px0, py0, px1, py1);
      }
    }
  }
}
