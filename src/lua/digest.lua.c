const char digest_lua[] =
"-- digest.lua (internal file)\n"
"\n"
"local ffi = require 'ffi'\n"
"\n"
"ffi.cdef[[\n"
"    /* from openssl/sha.h */\n"
"    unsigned char *SHA(const unsigned char *d, size_t n, unsigned char *md);\n"
"    unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md);\n"
"    unsigned char *SHA224(const unsigned char *d, size_t n,unsigned char *md);\n"
"    unsigned char *SHA256(const unsigned char *d, size_t n,unsigned char *md);\n"
"    unsigned char *SHA384(const unsigned char *d, size_t n,unsigned char *md);\n"
"    unsigned char *SHA512(const unsigned char *d, size_t n,unsigned char *md);\n"
"    unsigned char *MD4(const unsigned char *d, size_t n, unsigned char *md);\n"
"    unsigned char *SHA1internal(const unsigned char *d, size_t n, unsigned char *md);\n"
"\n"
"    /* from openssl/md5.h */\n"
"    unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md);\n"
"\n"
"    /* from libc */\n"
"    int snprintf(char *str, size_t size, const char *format, ...);\n"
"\n"
"    typedef uint32_t (*crc32_func)(uint32_t crc,\n"
"        const unsigned char *buf, unsigned int len);\n"
"    extern int32_t guava(int64_t state, int32_t buckets);\n"
"    extern crc32_func crc32_calc;\n"
"\n"
"    /* base64 */\n"
"    int base64_bufsize(int binsize);\n"
"    int base64_decode(const char *in_base64, int in_len, char *out_bin, int out_len);\n"
"    int base64_encode(const char *in_bin, int in_len, char *out_base64, int out_len);\n"
"\n"
"    /* random */\n"
"    void random_bytes(char *, size_t);\n"
"\n"
"    /* from third_party/PMurHash.h */\n"
"    void PMurHash32_Process(uint32_t *ph1, uint32_t *pcarry, const void *key, int len);\n"
"    uint32_t PMurHash32_Result(uint32_t h1, uint32_t carry, uint32_t total_length);\n"
"    uint32_t PMurHash32(uint32_t seed, const void *key, int len);\n"
"]]\n"
"\n"
"local ssl\n"
"if ssl == nil then\n"
"    local variants = {\n"
"        'libssl.so.10',\n"
"        'libssl.so.1.0.0',\n"
"        'libssl.so.0.9.8',\n"
"        'libssl.so',\n"
"        'ssl',\n"
"    }\n"
"\n"
"    for _, libname in pairs(variants) do\n"
"        pcall(function() ssl = ffi.load(libname) end)\n"
"        if ssl ~= nil then\n"
"            break\n"
"        end\n"
"    end\n"
"end\n"
"\n"
"local def = {\n"
"    sha     = { 'SHA',    20 },\n"
"    sha224  = { 'SHA224', 28 },\n"
"    sha256  = { 'SHA256', 32 },\n"
"    sha384  = { 'SHA384', 48 },\n"
"    sha512  = { 'SHA512', 64 },\n"
"    md5     = { 'MD5',    16 },\n"
"    md4     = { 'MD4',    16 },\n"
"}\n"
"\n"
"local hexres = ffi.new('char[129]')\n"
"\n"
"local function tohex(r, size)\n"
"    for i = 0, size - 1 do\n"
"        ffi.C.snprintf(hexres + i * 2, 3, \"%02x\",\n"
"            ffi.cast('unsigned int', r[i]))\n"
"    end\n"
"    return ffi.string(hexres, size * 2)\n"
"end\n"
"\n"
"local PMurHash\n"
"local PMurHash_methods = {\n"
"\n"
"    update = function(self, str)\n"
"        str = tostring(str or '')\n"
"        ffi.C.PMurHash32_Process(self.seed, self.value, str, string.len(str))\n"
"        self.total_length = self.total_length + string.len(str)\n"
"    end,\n"
"\n"
"    result = function(self)\n"
"        return ffi.C.PMurHash32_Result(self.seed[0], self.value[0], self.total_length)\n"
"    end,\n"
"\n"
"    clear = function(self)\n"
"        self.seed[0] = self.default_seed\n"
"        self.total_length = 0\n"
"        self.value[0] = 0\n"
"    end,\n"
"\n"
"    copy = function(self)\n"
"        local new_self = PMurHash.new()\n"
"        new_self.seed[0] = self.seed[0]\n"
"        new_self.value[0] = self.value[0]\n"
"        new_self.total_length = self.total_length\n"
"        return new_self\n"
"    end\n"
"}\n"
"\n"
"PMurHash = {\n"
"    default_seed = 13,\n"
"\n"
"    new = function(opts)\n"
"        opts = opts or {}\n"
"        local self = setmetatable({}, { __index = PMurHash_methods })\n"
"        self.default_seed = (opts.seed or PMurHash.default_seed)\n"
"        self.seed = ffi.new(\"int[1]\", self.default_seed)\n"
"        self.value = ffi.new(\"int[1]\", 0)\n"
"        self.total_length = 0\n"
"        return self\n"
"    end\n"
"}\n"
"\n"
"setmetatable(PMurHash, {\n"
"    __call = function(self, str)\n"
"        str = tostring(str or '')\n"
"        return ffi.C.PMurHash32(PMurHash.default_seed, str, string.len(str))\n"
"    end\n"
"})\n"
"\n"
"local CRC32\n"
"local CRC32_methods = {\n"
"    update = function(self, str)\n"
"        str = tostring(str or '')\n"
"        self.value = ffi.C.crc32_calc(self.value, str, string.len(str))\n"
"    end,\n"
"\n"
"    result = function(self)\n"
"        return self.value\n"
"    end,\n"
"\n"
"    clear = function(self)\n"
"        self.value = CRC32.crc_begin\n"
"    end,\n"
"\n"
"    copy = function(self)\n"
"        local new_self = CRC32.new()\n"
"        new_self.value = self.value\n"
"        return new_self\n"
"    end\n"
"}\n"
"\n"
"CRC32 = {\n"
"    crc_begin = 4294967295,\n"
"\n"
"    new = function()\n"
"        local self = setmetatable({}, { __index = CRC32_methods })\n"
"        self.value = CRC32.crc_begin\n"
"        return self\n"
"    end\n"
"}\n"
"\n"
"setmetatable(CRC32, {\n"
"    __call = function(self, str)\n"
"        str = tostring(str or '')\n"
"        return ffi.C.crc32_calc(CRC32.crc_begin, str, string.len(str))\n"
"    end\n"
"})\n"
"\n"
"local m = {\n"
"    base64_encode = function(bin)\n"
"        if type(bin) ~= 'string' then\n"
"            error('Usage: digest.base64_encode(string)')\n"
"        end\n"
"        local blen = #bin\n"
"        local slen = ffi.C.base64_bufsize(blen)\n"
"        local str  = ffi.new('char[?]', slen)\n"
"        local len = ffi.C.base64_encode(bin, blen, str, slen)\n"
"        return ffi.string(str, len)\n"
"    end,\n"
"\n"
"    base64_decode = function(str)\n"
"        if type(str) ~= 'string' then\n"
"            error('Usage: digest.base64_decode(string)')\n"
"        end\n"
"        local slen = #str\n"
"        local blen = math.ceil(slen * 3 / 4)\n"
"        local bin  = ffi.new('char[?]', blen)\n"
"        local len = ffi.C.base64_decode(str, slen, bin, blen)\n"
"        return ffi.string(bin, len)\n"
"    end,\n"
"\n"
"    crc32 = CRC32,\n"
"\n"
"    crc32_update = function(crc, str)\n"
"        str = tostring(str or '')\n"
"        return ffi.C.crc32_calc(tonumber(crc), str, string.len(str))\n"
"    end,\n"
"\n"
"    sha1 = function(str)\n"
"        if str == nil then\n"
"            str = ''\n"
"        else\n"
"            str = tostring(str)\n"
"        end\n"
"        local r = ffi.C.SHA1internal(str, #str, nil)\n"
"        return ffi.string(r, 20)\n"
"    end,\n"
"\n"
"    sha1_hex = function(str)\n"
"        if str == nil then\n"
"            str = ''\n"
"        else\n"
"            str = tostring(str)\n"
"        end\n"
"        local r = ffi.C.SHA1internal(str, #str, nil)\n"
"        return tohex(r, 20)\n"
"    end,\n"
"\n"
"    guava = function(state, buckets)\n"
"       return ffi.C.guava(state, buckets)\n"
"    end,\n"
"\n"
"    urandom = function(n)\n"
"        if n == nil then\n"
"            error('Usage: digest.urandom(len)')\n"
"        end\n"
"        local buf = ffi.new('char[?]', n)\n"
"        ffi.C.random_bytes(buf, n)\n"
"        return ffi.string(buf, n)\n"
"    end,\n"
"\n"
"    murmur = PMurHash\n"
"}\n"
"\n"
"if ssl ~= nil then\n"
"\n"
"    for pname, df in pairs(def) do\n"
"        local hfunction = df[1]\n"
"        local hsize = df[2]\n"
"\n"
"        m[ pname ] = function(str)\n"
"            if str == nil then\n"
"                str = ''\n"
"            else\n"
"                str = tostring(str)\n"
"            end\n"
"            local r = ssl[hfunction](str, string.len(str), nil)\n"
"            return ffi.string(r, hsize)\n"
"        end\n"
"\n"
"        m[ pname .. '_hex' ] = function(str)\n"
"            if str == nil then\n"
"                str = ''\n"
"            else\n"
"                str = tostring(str)\n"
"            end\n"
"            local r = ssl[hfunction](str, string.len(str), nil)\n"
"            return tohex(r, hsize)\n"
"        end\n"
"    end\n"
"else\n"
"    local function errorf()\n"
"        error(\"libSSL was not loaded\")\n"
"    end\n"
"    for pname, df in pairs(def) do\n"
"        m[ pname ] = errorf\n"
"        m[ pname .. '_hex' ] = errorf\n"
"    end\n"
"end\n"
"\n"
"return m\n"
""
;
