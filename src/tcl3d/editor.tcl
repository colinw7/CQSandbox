proc init { } {
  sb3d::ui create "\
<qxml>\n\
<QGroupBox title=\"Add\">
<QVBoxLayout>
<QPushButton text=\"BBox\" onClicked=\"addBBoxProc\"/>\n\
<QPushButton text=\"Cone\" onClicked=\"addConeProc\"/>\n\
<QPushButton text=\"Cube\" onClicked=\"addCubeProc\"/>\n\
<QPushButton text=\"Cylinder\" onClicked=\"addCylinderProc\"/>\n\
<QPushButton text=\"Sphere\" onClicked=\"addSphereProc\"/>\n\
<QPushButton text=\"Plane\" onClicked=\"addPlaneProc\"/>\n\
</QVBoxLayout>
</QGroupBox>
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc addBBoxProc { } {
  sb3d::bbox
}

proc addConeProc { } {
  set shape [sb3d::shape]

  $shape set cone 1.0 1.0
}

proc addCubeProc { } {
  sb3d::cube
}

proc addCylinderProc { } {
  set shape [sb3d::shape]
  
  $shape set cylinder 1.0 1.0
}

proc addSphereProc { } {
  set shape [sb3d::shape]

  $shape set sphere 1.0
}

proc addPlaneProc { } {
  sb3d::plane
}
