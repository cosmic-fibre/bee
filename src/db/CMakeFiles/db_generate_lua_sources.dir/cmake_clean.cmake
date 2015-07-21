FILE(REMOVE_RECURSE
  "lua/load_cfg.lua.c"
  "lua/schema.lua.c"
  "lua/tuple.lua.c"
  "lua/session.lua.c"
  "lua/snapshot_daemon.lua.c"
  "CMakeFiles/db_generate_lua_sources"
  "lua/load_cfg.lua.c"
  "lua/schema.lua.c"
  "lua/tuple.lua.c"
  "lua/session.lua.c"
  "lua/snapshot_daemon.lua.c"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/db_generate_lua_sources.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
