-- db.uuid
uuid = require('uuid')

--
-- RFC4122 compliance
--
uu = uuid.new()
-- new()always generates RFC4122 variant
bit.band(uu.clock_seq_hi_and_reserved, 0xc0) == 0x80
vsn = bit.rshift(uu.time_hi_and_version, 12)
-- new() generates time-based or random-based version
vsn == 1 or vsn == 4

--
-- to/from string
--
uu = uuid()
#uu:str()
string.match(uu:str(), '^[a-f0-9%-]+$') ~= nil
uu == uuid.fromstr(uu:str())
uu = uuid.fromstr('ba90d815-14e0-431d-80c0-ce587885bb78')
uu:str()
tostring(uu)
tostring(uu) == uu:str()
uu.time_low;
uu.time_mid;
uu.time_hi_and_version;
uu.clock_seq_hi_and_reserved;
uu.clock_seq_low;
uu.node[0]
uu.node[1]
uu.node[2]
uu.node[3]
uu.node[4]
uu.node[5]

-- aliases
#uuid.str()

-- invalid values
uuid.fromstr(nil)
uuid.fromstr('')
uuid.fromstr('blablabla')
uuid.fromstr(string.rep(' ', 36))
uuid.fromstr('ba90d81514e0431d80c0ce587885bb78')
uuid.fromstr('ba90d815-14e0-431d-80c0')
uuid.fromstr('ba90d815-14e0-431d-80c0-tt587885bb7')

--
-- to/from binary
--
uu = uuid()
#uu:bin()
#uu:bin('h')
#uu:bin('l')
#uu:bin('n')
#uu:bin('b')
uu:bin() == uu:bin('h')
uu:bin('n') ~= uu:bin('h')
uu:bin('b') ~= uu:bin('l')
uu == uuid.frombin(uu:bin())
uu == uuid.frombin(uu:bin('b'), 'b')
uu == uuid.frombin(uu:bin('l'), 'l')

uu = uuid.fromstr('adf9d02e-0756-11e4-b5cf-525400123456')
uu:bin('l')
uu:bin('b')

-- aliases
#uuid.bin()
#uuid.bin('l')

--
-- eq and nil
--

uu = uuid.new()
uuid.NULL
uuid.NULL:isnil()
uuid.NULL ~= uu
uu:isnil()
uu ==  uu
uu == uu
uu == nil
uu == 12345
uu == "blablabla"

--
-- invalid usage
--

uu = uuid.new()
uu.isnil()
uu.bin()
uu.str()

uu = nil
uuid = nil
