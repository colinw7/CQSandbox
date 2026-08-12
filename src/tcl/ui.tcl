proc init { } {
  puts "init"

  sb::canvas set range {0 0 100 100}

  set ::path [sb::path {M 25 25 L 50 25 L 50 50 L 75 50}]

  sb::ui "\
<qxml>\n\
<QPushButton text=\"Button 1\" onClicked=\"button1Proc\"/>\n\
<QPushButton text=\"Button 2\" onClicked=\"button2Proc\"/>\n\
<QPushButton text=\"Button 3\" onClicked=\"button3Proc\"/>\n\
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc button1Proc { } {
  echo "button1Proc"
}

proc button2Proc { } {
  echo "button2Proc"
}

proc button3Proc { } {
  echo "button3Proc"
}
