proc printMaterial { material } {
  echo "Material: $material"
  echo "  Diffuse: [$material get diffuse]"
}

proc init { } {
  set ::model_dir models/obj
  sb3d::canvas set model_dir $::model_dir
  set ::model [sb3d::model models/obj/tommygun.obj]

  set materials [sb3d::canvas get materials]

  foreach material $materials {
    printMaterial $material
  }

  set material1 [sb3d::material]

  printMaterial $material1
}
