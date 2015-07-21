FILE(REMOVE_RECURSE
  "CMakeFiles/man-bee"
  "bee.1"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/man-bee.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
