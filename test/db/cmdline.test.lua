
arg[-1] ~= nil
arg[0] ~= nil
string.match(arg[-1], '^/') ~= nil
string.match(arg[0], '^/') ~= nil

string.match(arg[-1], '/bee$') ~= nil
string.match(arg[0], '/db%.lua$') ~= nil

io.type( io.open(arg[-1]) )
io.type( io.open(arg[0]) )

