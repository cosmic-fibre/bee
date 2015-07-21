FILE(REMOVE_RECURSE
  "lua/init.lua.c"
  "lua/fiber.lua.c"
  "lua/uuid.lua.c"
  "lua/digest.lua.c"
  "lua/msgpackffi.lua.c"
  "lua/uri.lua.c"
  "lua/console.lua.c"
  "lua/bsdsocket.lua.c"
  "lua/errno.lua.c"
  "lua/log.lua.c"
  "lua/net_db.lua.c"
  "lua/help.lua.c"
  "lua/help_en_US.lua.c"
  "lua/tap.lua.c"
  "lua/fio.lua.c"
  "../third_party/luafun/fun.lua.c"
  "../third_party/astr/astr.lua.c"
  "../third_party/tools/shard.lua.c"
  "../third_party/tools/csv.lua.c"
  "../third_party/expiration/expirationd.lua.c"
  "../third_party/sodium/crypto_init.lua.c"
  "../third_party/sodium/crypto_auth.lua.c"
  "../third_party/sodium/crypto_box.lua.c"
  "../third_party/sodium/crypto_core.lua.c"
  "../third_party/sodium/crypto_generichash.lua.c"
  "../third_party/sodium/crypto_hash.lua.c"
  "../third_party/sodium/crypto_hashblocks.lua.c"
  "../third_party/sodium/crypto_onetimeauth.lua.c"
  "../third_party/sodium/crypto_scalarmult.lua.c"
  "../third_party/sodium/crypto_secretbox.lua.c"
  "../third_party/sodium/crypto_shorthash.lua.c"
  "../third_party/sodium/crypto_secretbox.lua.c"
  "../third_party/sodium/crypto_sign.lua.c"
  "../third_party/sodium/crypto_stream.lua.c"
  "../third_party/sodium/crypto_verify.lua.c"
  "../third_party/sodium/randombytes.lua.c"
  "../third_party/config/cfg.lua.c"
  "CMakeFiles/bee.dir/main.cc.o"
  "bee.pdb"
  "bee"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang CXX)
  INCLUDE(CMakeFiles/bee.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
