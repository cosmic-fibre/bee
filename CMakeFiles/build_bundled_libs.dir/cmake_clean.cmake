FILE(REMOVE_RECURSE
  "CMakeFiles/build_bundled_libs"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/build_bundled_libs.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
