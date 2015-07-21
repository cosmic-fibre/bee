FILE(REMOVE_RECURSE
  "CMakeFiles/libsham"
  "third_party/sham/libsham.a"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/libsham.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
