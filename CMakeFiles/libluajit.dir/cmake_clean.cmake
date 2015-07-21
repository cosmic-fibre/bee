FILE(REMOVE_RECURSE
  "CMakeFiles/libluajit"
  "third_party/luajit/src/libluajit.a"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/libluajit.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
