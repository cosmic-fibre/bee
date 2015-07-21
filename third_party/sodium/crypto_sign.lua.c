const char crypto_sign_lua[] =
"local ffi = require \"ffi\"\n"
"local lib = ffi.load \"sodium\"\n"
"\n"
"ffi.cdef [[\n"
"size_t crypto_sign_ed25519_bytes(void);\n"
"size_t crypto_sign_ed25519_seedbytes(void);\n"
"size_t crypto_sign_ed25519_publickeybytes(void);\n"
"size_t crypto_sign_ed25519_secretkeybytes(void);\n"
"const char * crypto_sign_ed25519_primitive(void);\n"
"int crypto_sign_ed25519(unsigned char *,unsigned long long *,const unsigned char *,unsigned long long,const unsigned char *);\n"
"int crypto_sign_ed25519_open(unsigned char *,unsigned long long *,const unsigned char *,unsigned long long,const unsigned char *);\n"
"int crypto_sign_ed25519_keypair(unsigned char *,unsigned char *);\n"
"int crypto_sign_ed25519_seed_keypair(unsigned char *,unsigned char *,const unsigned char *);\n"
"size_t crypto_sign_bytes(void);\n"
"size_t crypto_sign_seedbytes(void);\n"
"size_t crypto_sign_publickeybytes(void);\n"
"size_t crypto_sign_secretkeybytes(void);\n"
"const char *crypto_sign_primitive(void);\n"
"int crypto_sign_seed_keypair(unsigned char *pk, unsigned char *sk,\n"
"                             const unsigned char *seed);\n"
"int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);\n"
"int crypto_sign(unsigned char *sm, unsigned long long *smlen,\n"
"                const unsigned char *m, unsigned long long mlen,\n"
"                const unsigned char *sk);\n"
"int crypto_sign_open(unsigned char *m, unsigned long long *mlen,\n"
"                     const unsigned char *sm, unsigned long long smlen,\n"
"                     const unsigned char *pk);\n"
"size_t crypto_sign_edwards25519sha512batch_bytes(void);\n"
"size_t crypto_sign_edwards25519sha512batch_publickeybytes(void);\n"
"size_t crypto_sign_edwards25519sha512batch_secretkeybytes(void);\n"
"const char * crypto_sign_edwards25519sha512batch_primitive(void);\n"
"int crypto_sign_edwards25519sha512batch(unsigned char *,unsigned long long *,const unsigned char *,unsigned long long,const unsigned char *);\n"
"int crypto_sign_edwards25519sha512batch_open(unsigned char *,unsigned long long *,const unsigned char *,unsigned long long,const unsigned char *);\n"
"int crypto_sign_edwards25519sha512batch_keypair(unsigned char *,unsigned char *);\n"
"]]\n"
"\n"
"local _M = { }\n"
"\n"
"_M.bytes = function() return tonumber(lib.crypto_sign_bytes()) end\n"
"_M.seedbytes = function() return tonumber(lib.crypto_sign_seedbytes()) end\n"
"_M.publickeybytes = function() return tonumber(lib.crypto_sign_publickeybytes()) end\n"
"_M.secretkeybytes = function() return tonumber(lib.crypto_sign_secretkeybytes()) end\n"
"_M.primitive = function() return ffi.string(lib.crypto_sign_primitive()) end\n"
"_M.seed_keypair = lib.crypto_sign_seed_keypair\n"
"_M.keypair = function(pk, sk)\n"
"\tpk = pk or ffi.new(\"unsigned char[?]\", _M.publickeybytes())\n"
"\tsk = sk or ffi.new(\"unsigned char[?]\", _M.secretkeybytes())\n"
"\tassert(lib.crypto_sign_keypair(pk, sk) == 0, \"crypto_sign_keypairfunction returned non-zero\")\n"
"\treturn pk, sk\n"
"end\n"
"_M.sign = function(sm, smlen, m, mlen, sk)\n"
"\tmlen = mlen or #m\n"
"\tsm = sm or ffi.new(\"unsigned char[?]\", mlen + _M.bytes())\n"
"\tsmlen = smlen or ffi.new(\"unsigned long long[1]\")\n"
"\tassert(lib.crypto_sign(sm, smlen, m, mlen, sk) == 0, \"crypto_sign returned non-zero\")\n"
"\treturn sm, smlen[0]\n"
"end\n"
"_M.open = function(m, mlen, sm, smlen, pk)\n"
"\tsmlen = smlen or #sm\n"
"\tm = m or ffi.new(\"unsigned char[?]\", smlen)\n"
"\tmlen = mlen or ffi.new(\"unsigned long long[1]\")\n"
"\tassert(lib.crypto_sign_open(m, mlen, sm, smlen, pk) == 0, \"crypto_sign_open returned non-zero\")\n"
"\treturn m, mlen[0]\n"
"end\n"
"\n"
"return setmetatable ( _M , { __call = function(_M, ...) return _M.sign(...) end } )\n"
""
;
