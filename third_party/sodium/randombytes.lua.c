const char randombytes_lua[] =
"local ffi = require \"ffi\"\n"
"local lib = ffi.load \"sodium\"\n"
"\n"
"ffi.cdef [[\n"
"typedef struct randombytes_implementation {\n"
"    const char *(*implementation_name)(void);\n"
"    uint32_t (*random)(void);\n"
"    void (*stir)(void);\n"
"    uint32_t (*uniform)(const uint32_t upper_bound);\n"
"    void (*buf)(void * const buf, const size_t size);\n"
"    int (*close)(void);\n"
"} randombytes_implementation;\n"
"int randombytes_set_implementation(randombytes_implementation *impl);\n"
"void randombytes(unsigned char * const buf, const unsigned long long buf_len);\n"
"const char *randombytes_implementation_name(void);\n"
"uint32_t randombytes_random(void);\n"
"void randombytes_stir(void);\n"
"uint32_t randombytes_uniform(const uint32_t upper_bound);\n"
"void randombytes_buf(void * const buf, const size_t size);\n"
"int randombytes_close(void);\n"
"extern struct randombytes_implementation randombytes_salsa20_implementation;\n"
"const char *randombytes_salsa20_implementation_name(void);\n"
"uint32_t randombytes_salsa20_random(void);\n"
"void randombytes_salsa20_random_stir(void);\n"
"uint32_t randombytes_salsa20_random_uniform(const uint32_t upper_bound);\n"
"void randombytes_salsa20_random_buf(void * const buf, const size_t size);\n"
"int randombytes_salsa20_random_close(void);\n"
"extern struct randombytes_implementation randombytes_sysrandom_implementation;\n"
"const char *randombytes_sysrandom_implementation_name(void);\n"
"uint32_t randombytes_sysrandom(void);\n"
"void randombytes_sysrandom_stir(void);\n"
"uint32_t randombytes_sysrandom_uniform(const uint32_t upper_bound);\n"
"void randombytes_sysrandom_buf(void * const buf, const size_t size);\n"
"int randombytes_sysrandom_close(void);\n"
"]]\n"
"\n"
"local _M = { }\n"
"\n"
"_M.random = function() return tonumber(lib.randombytes_random()) end\n"
"_M.stir = lib.randombytes_stir\n"
"_M.uniform = function(upper) return tonumber(lib.randombytes_uniform(upper)) end\n"
"_M.buf = function(buf, size)\n"
"\tbuf = buf or ffi.new(\"unsigned char[?]\", size)\n"
"\tlib.randombytes_buf(buf, size)\n"
"\treturn buf, size\n"
"end\n"
"_M.close = function()\n"
"\tassert(lib.randombytes_close() == 0, \"randombytes_close returned non-zero\")\n"
"end\n"
"\n"
"return _M\n"
""
;
