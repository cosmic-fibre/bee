const char crypto_generichash_lua[] =
"local ffi = require \"ffi\"\n"
"local lib = ffi.load \"sodium\"\n"
"\n"
"ffi.cdef [[\n"
"#pragma pack(push, 1)\n"
"__attribute__((aligned(64))) typedef struct crypto_generichash_blake2b_state {\n"
"    uint64_t h[8];\n"
"    uint64_t t[2];\n"
"    uint64_t f[2];\n"
"    uint8_t buf[2 * 128U];\n"
"    size_t buflen;\n"
"    uint8_t last_node;\n"
"} crypto_generichash_blake2b_state;\n"
"#pragma pack(pop)\n"
"size_t crypto_generichash_blake2b_bytes_min(void);\n"
"size_t crypto_generichash_blake2b_bytes_max(void);\n"
"size_t crypto_generichash_blake2b_keybytes_min(void);\n"
"size_t crypto_generichash_blake2b_keybytes_max(void);\n"
"size_t crypto_generichash_blake2b_blockbytes(void);\n"
"const char * crypto_generichash_blake2b_blockbytes_primitive(void);\n"
"int crypto_generichash_blake2b(unsigned char *out, size_t outlen,\n"
"                               const unsigned char *in,\n"
"                               unsigned long long inlen,\n"
"                               const unsigned char *key, size_t keylen);\n"
"int crypto_generichash_blake2b_init(crypto_generichash_blake2b_state *state,\n"
"                                    const unsigned char *key,\n"
"                                    const size_t keylen, const size_t outlen);\n"
"int crypto_generichash_blake2b_update(crypto_generichash_blake2b_state *state,\n"
"                                      const unsigned char *in,\n"
"                                      unsigned long long inlen);\n"
"int crypto_generichash_blake2b_final(crypto_generichash_blake2b_state *state,\n"
"                                     unsigned char *out,\n"
"                                     const size_t outlen);\n"
"size_t crypto_generichash_bytes(void);\n"
"size_t crypto_generichash_bytes_min(void);\n"
"size_t crypto_generichash_bytes_max(void);\n"
"size_t crypto_generichash_keybytes(void);\n"
"size_t crypto_generichash_keybytes_min(void);\n"
"size_t crypto_generichash_keybytes_max(void);\n"
"size_t crypto_generichash_blockbytes(void);\n"
"const char *crypto_generichash_primitive(void);\n"
"typedef crypto_generichash_blake2b_state crypto_generichash_state;\n"
"int crypto_generichash(unsigned char *out, size_t outlen,\n"
"                       const unsigned char *in, unsigned long long inlen,\n"
"                       const unsigned char *key, size_t keylen);\n"
"int crypto_generichash_init(crypto_generichash_state *state,\n"
"                            const unsigned char *key,\n"
"                            const size_t keylen, const size_t outlen);\n"
"int crypto_generichash_update(crypto_generichash_state *state,\n"
"                              const unsigned char *in,\n"
"                              unsigned long long inlen);\n"
"int crypto_generichash_final(crypto_generichash_state *state,\n"
"                             unsigned char *out, const size_t outlen);\n"
"\n"
"]]\n"
"\n"
"local _M = { }\n"
"\n"
"_M.bytes = function() return tonumber(crypto_generichash_bytes()) end\n"
"_M.bytes_min = function() return tonumber(crypto_generichash_bytes_min()) end\n"
"_M.bytes_max = function() return tonumber(crypto_generichash_bytes_max()) end\n"
"_M.keybytes = function() return tonumber(crypto_generichash_keybytes()) end\n"
"_M.keybytes_min = function() return tonumber(crypto_generichash_keybytes_min()) end\n"
"_M.keybytes_max = function() return tonumber(crypto_generichash_keybytes_max()) end\n"
"_M.blockbytes = function() return tonumber(crypto_generichash_blockbytes()) end\n"
"_M.primitive = function() return ffi.string(lib.crypto_generichash_primitive()) end\n"
"_M.generichash = function(out, outlen, inbuf, inlen, key, keylen)\n"
"\tout = out or ffi.new(\"unsigned char[?]\", outlen)\n"
"\tinlen = inlen or #inbuf\n"
"\tkeylen = keylen or (key and #key or 0)\n"
"\tassert(lib.crypto_generichash(out, outlen, inbuf, inlen, key, keylen) == 0, \"crypto_generichash returned non-zero\")\n"
"\treturn out, outlen\n"
"end\n"
"_M.init = function(state, key, keylen, outlen)\n"
"\tkeylen = keylen or (key and #key or 0)\n"
"\tassert(lib.crypto_generichash_init(state, key, keylen, outlen) == 0, \"crypto_generichash_init returned non-zero\")\n"
"\treturn state\n"
"end\n"
"_M.update = function(state, inbuf, inlen)\n"
"\tinlen = inlen or #inbuf\n"
"\tassert(lib.crypto_generichash_update(state, inbuf, inlen) == 0, \"crypto_generichash_update returned non-zero\")\n"
"\treturn state\n"
"end\n"
"_M.final = function(state, out, outlen)\n"
"\tout = out or ffi.new(\"unsigned char[?]\", outlen)\n"
"\tassert(lib.crypto_generichash_final(state, out, outlen) == 0, \"crypto_generichash_final returned non-zero\")\n"
"\treturn out, outlen\n"
"end\n"
"_M.stream = ffi.metatype(\"struct crypto_generichash_blake2b_state\", {\n"
"\t__index = {\n"
"\t\tinit = _M.init ;\n"
"\t\tupdate = _M.update ;\n"
"\t\tfinal = _M.final ;\n"
"\t} ;\n"
"})\n"
"\n"
"return setmetatable ( _M , { __call = function(_M, ...) return _M.generichash(...) end } )\n"
""
;
