--- help (en_US)

return {
    tutorial = {
    [[
Tutorial -- Screen #1 -- Hello, Moon
====================================

Welcome to the Bee tutorial.
It will introduce you to Bee’s Lua application server
and database server, which is what’s running what you’re seeing.
This is INTERACTIVE -- you’re expected to enter requests
based on the suggestions or examples in the screen’s text.

The first request is:

5.1, "Olá", "Lua"
------------------

This isn’t your grandfather’s "Hello World" ...
the decimal literal 5.1 and the two strings inside
single quotes ("Hello Moon" in Portuguese) will just
be repeated, without need for a print() statement.

Take that one-line request and enter it below after the
"bee>" prompt, then type Enter.
You’ll see the response:
---
- 5.1
- Olá
- Lua
...
Then you’ll get a chance to repeat -- perhaps entering
something else such as "Longer String",-1,-3,0.
When you’re ready to go to the next screen, enter <tutorial("next")>.
]];

[[
Tutorial -- Screen #2 -- Variables
==================================


Improve on "5.1, "Olá", "Lua""
by using variables rather than literals,
and put the strings inside braces, which means
they’re elements of a TABLE.
More in the Lua manual: http://www.lua.org/pil/2.html

You don’t need to declare variables in advance
because Lua figures out the data type from what
you assign to it. Assignment is done with the "=" operator.

If the data type of variable t is table, then the
elements can be referenced as t[1], t[2], and so on.
More in the Lua manual: http://www.lua.org/pil/1.2.html

Request #2 is:

n = 5.1
t = {"Olá", "Lua"}
n, t[1], t[2]
------------------

Take all three lines and enter them below after the
"bee>" prompt, then type Enter.
Or try different values in a different order.
When you’re ready to go to the next screen, enter <tutorial("next")>.
]];

[[
Tutorial -- Screen #3 -- Loops
==============================

Add action by using loops rather than static displays.
There are several syntaxes for loops in Lua,
but we’ll just use one:
for variable-name = start-value, end-value, 1 do loop-body end
which is good enough if you want to assign a
start-value to a variable, do what’s in the loop body,
add 1 to the variable, and repeat until it equals end-value.
More in the Lua manual: http://www.lua.org/pil/4.3.4.html.

Request #3 is:

result_table = {}
n = 5.1
for i=1,2,1 do result_table[i] = n * i end
result_table
--------------------------------------------------------------

Take all four lines and enter them below after the
"bee>" prompt, then type Enter.
For adventure, change the loop to "for i=1,3,1"
(don’t worry, it won’t crash).
When you’re ready to go to the next screen, enter <tutorial("next")>.
]];

[[
Tutorial -- Screen #4 -- Operators
==================================

Among the many operators that Lua supports, you most often see:
For arithmetic: * (multiply), + (add), - (subtract), / (divide).
For strings: .. (concatenate)
More in the Lua manual: http://www.lua.org/pil/3.1.html

Request #4 is:

n = 5.1
t = {"Olá", "Lua"}
for i=1,2,1 do n = n * 2 t[1] = t[1] .. t[2] end
n,t[1],t[2]
------------------------------------------------

Before you type that in and see Bee display the result,
try to predict whether the display will be
(a) 20.4 OláLuaLua Lua
(b) 10.2 Olá Lua Lua Lua
(c) 5.1 Olá Lua

The answer will appear when you type in the request.
When you’re ready to go to the next screen, enter <tutorial("next")>.
]];

[[
Tutorial -- Screen #5 -- Conditions
===================================

A condition involves a comparison operator such as "==",
">", ">=", "<", "<=". Conditions are used in statements
of the form if ... then.
More in the Lua manual: http://www.lua.org/pil/4.3.1.html

Request #5 is:

x = 17
if x * 2 > 34 then result = x else result = "no" end
result
------------------------------------------------

Before you type in those three lines and see Bee display
the result, try to predict whether the display will be
(a) 17
(b) 34
(c) no
The answer will appear when you type in the request.
When you’re ready to go to the next screen, enter <tutorial("next")>.
]];

[[
Tutorial -- Screen #6 -- Delimiters
===================================

This is just to prepare for later exercises
which will go over many lines. There is a
Bee instruction that means <don’t execute
every time I type Enter; wait until I type a
special string called the "delimiter".>
More in the Bee manual: http://bee.org/doc/master/user_guide.html#utility-bee-delim

Request #6 is:

console = require("console"); console.delimiter("!")
-----------------------------------------------------

It’s not an exercise -- just do it.
Cancelling the delimiter could be done with
console.delimiter("")!
but you’ll see "!" in following exercises.

When you’re ready to go to the next screen, enter <tutorial("next")!>.
Yes, <tutorial("next")!> has to end with an exclamation mark too!
]];

[[
Tutorial -- Screen #7 -- Simple Functions
=========================================

A function, or a stored procedure that returns a value,
is a named set of Lua requests whose simplest form is
function function_name () body end
More in the Lua manual: http://www.lua.org/pil/5.html

Request #7 is:
n = 0
function func ()
for i=1,100,1 do n = n + i end
return n
end!
func()!
--------------------------------------------------

This defines a function which sums all the numbers
between 1 and 100, and returns the final result.
The request "func()!" invokes the function.
]];

[[
Tutorial -- Screen #8 -- Improved Functions
===========================================

Improve the simple function by avoiding globals.
The variable n could be passed as a parameter
and the variable i could be declared as local.
More in the Lua manual: http://www.lua.org/pil/4.2.html

Request #8 is:

function func (n)
local i
for i=1,100,1 do n = n + i end
return n
end!
func(0)!
------------------------------------
]];

[[
Tutorial -- Screen #9 -- Comments
=================================

There are several ways to add comments, but
one will do: (minus sign) (minus sign) comment-text.
More in the Lua manual: http://www.lua.org/pil/1.3.html

Request #9 is:

-- func is a function which returns a sum.
-- n is a parameter. i is a local variable.
-- "!" is a delimiter (introduced in Screen #6)
-- func is a function (introduced in Screen #7)
-- n is a parameter (introduced in Screen #8)
-- "n = n + 1" is an operator usage (introduced in Screen #4)
-- "for ... do ... end" is a loop (introduced in Screen #3)
function func(n) -- n is a parameter
local i -- i is a local variable
for i=1,100,1 do n = n + i end
return n
end!
-- invoke the function
func(0)!
-------------------------------------------

Obviously it will work, so just type <tutorial("next")!> now.
]];

[[
Tutorial -- Screen #10 -- Packages
==================================

Many developers have gone to the trouble of making
packages of functions (sometimes called "modules")
that have a general utility.
More in the Luarocks list: http://luarocks.org/

Most packages have to be "required", with the syntax
variable_name = require("package-name")
which should look familiar because earlier you said
console = require("console")

At this point, if you just say the variable_name,
you’ll see a list of the package’s members and
functions. If then you use a "." operator as in
package-variable-name . function_name()
you’ll invoke a package’s function.
(At a different level you’ll have to use a ":"
operator, as you’ll see in later examples.)

Request #10 is:

session = require("db.session")!
session!
session.user()!
---------------------------------

First you’ll see a list of functions, one of which
is "user". Then you’ll see the name of the current
user, which happens to be "admin", which happens to
be you.
]];

[[
Tutorial -- Screen #11 -- The socket package
============================================

Connect to the Internet and send a message to the parent of
Bee, the mail.ru corporation.

Request #11 is:

function socket_get ()
local socket, sock, result
socket = require("socket")
sock = socket.tcp_connect("mail.ru", 80)
sock:send("GET / HTTP/1.0\r\nHost: mail.ru\r\n\r\n")
result = sock:read(17)
sock:close()
return result
end!
socket_get()!
--------------------------------

Briefly these requests are opening a socket
and sending a "GET" request to mail.ru’s server.
The response will be short -- "- "HTTP/1.1 302 OK\r\n"" --
but it shows you’ve gotten in touch with a distant server.
More in the Bee manual:
http://bee.org/doc/master/user_guide.html#sp-db-socket
]];

[[
Tutorial -- Section #12 -- Fibers
=================================

Make a function that will run like a daemon in the
background until you cancel it. For this you need
a fiber. Bee is a "cooperative multitasking"
application server, which means that multiple
tasks each get a slice, but they have to yield
occasionally so that other tasks get a chance.
That’s what a properly designed fiber will do.
More in the Bee manual:
http://bee.org/doc/master/user_guide.html#sp-db-fiber

Request #12 is:

fiber = require("fiber")!
function function_x()
local i
gvar = 0
for i=0,600,1 do
gvar = gvar + 1
fiber.sleep(1)
end
end!
fid = fiber.create(function_x)!
gvar!
-----------------------------

The fiber.sleep(1) function will go to sleep for
one second, which is one way of yielding.
So the "for i=0,600,1" loop will go on for about 600 seconds (10 minutes).
During waking moments, gvar will go up by 1 -- and
gvar is deliberately a global variable. So it’s
possible to monitor it: slowly type "gvar!" a few
times and notice how the value mysteriously increases.
]];

[[
Tutorial -- Screen #13 -- Bee’s db package
=================================================

So far you’ve seen Bee in action as a
Lua application server. Henceforth you’ll see
it as a DBMS (database management system) server
-- with Lua stored procedures.

In serious situations you’d have to ask the
database administrator to create database objects
and granted read/write access to you, but here
you’re the "admin" user -- you have administrative
powers -- so you can start manipulating data immediately.
More in the Bee manual:
http://bee.org/doc/master/user_guide.html#databases

Request #13 is:

db.schema.create_space("tutor")!
db.space.tutor:create_index("primary",{})!
db.space.tutor:replace{1,"First tuple"}!
db.space.tutor:replace{2,"Second tuple"}!
db.space.tutor:replace{3,"Third tuple"}!
db.space.tutor:replace{4,"Fourth tuple"}!
db.space.tutor:replace{5,"Fifth tuple"}!
db.space.tutor!
----------------

Please ignore all the requests except the last one.
You’ll see a description of a space named tutor.
To understand the description, you just have to know that:
** fields are numbered item-storage areas
(vaguely like columns in an SQL DBMS)
** tuples are collections of fields, as are Lua tables
(vaguely like rows in an SQL DBMS)
** spaces are where Bee stores sets of tuples
(vaguely like databases in an SQL DBMS)
** indexes are objects that make lookups of tuples faster
(vaguely like indexes in an SQL DBMS)
Much of the description doesn’t matter right now; it’s
enough if you see that package db gets a space which is
named tutor, and it has one index on the first field.
]];

[[
Tutorial -- Screen #14 -- db.select()
======================================

The most common data-manipulation function is db.select().

One of the syntaxes is:
db.space.tutor.index.primary:select({1}, {iterator = "GE"})
and it returns a set of tuples via the index of the tutor
space.
Now that you know that, and considering that you already
know how to make functions and loops in Lua, it’s simple
to figure out how to search and display the first five
tuples in the database.

Request #14 is:

-- This function will select and display 5 tuples in space=tutor
function database_display (space_name)
local i
local result = ""
t = db.space[space_name].index.primary:select({1}, {iterator = "GE"})
for i=1,5,1 do
result = result .. t[i][1] .. " " .. t[i][2] .. "\n"
end
return result
end!
database_display("tutor")!
-----------------------------------------------------------------

So select() is returning a set of tuples into a Lua table
named t, and the loop is going to print each element of
the table. That is, when you call database_display()! you’ll
see a display of what’s in the tuples.
]];

[[
Tutorial -- Screen #15 -- db.replace()
=======================================

Pick any of the tuples that were displayed on the last screen.
Recall that the first field is the indexed field.
That’s all you need to replace the rest of the fields with
new values. The syntax of db.replace(), pared down, is:
db.space.tutor:replace{primary-key-field, other-fields}
More in the Bee manual:
http://bee.org/doc/master/user_guide.html#db.replace
Bee by default keeps database changes in memory,
but db.replace() will cause a write to a log, and log
information can later be consolidated with another db
function (db.snapshot).

Request #15 is:

db.space.tutor:replace{1, "My First Piece Of Data"}!
-----------------------------------------------------

If there is already a "tuple" (our equivalent of a record)
whose number is equal to 1, it will be replaced with your
new data. Otherwise it will be created for the first time.
The display will be the formal description of the new tuple.
]];

[[
Tutorial -- Screen #16 -- Create your own space
===============================================

You’ve now selected and replaced tuples from the
tutor space, and you could select and replace many
tuples because you know how to make variables and
functions and loops that do selecting or replacing.
But you’ve been confined to a space and an index
that Bee started with.
Suppose that you want to create your own.
More in the Bee manual:
http://bee.org/doc/master/user_guide.html#db.schema

Request #16 is:

db.schema.create_space("test", {engine="memtx"})!

--------------------------------------------------

The new space’s name will be "test" and the engine
will be "memtx" -- the engine which keeps all tuples
in memory, and writes changes to a log file to ensure
that data can’t be lost. Although "memtx" is the
default engine anyway, specifying it does no harm.
]];

[[
Tutorial -- Screen #17 -- Create your own index
===============================================

Having a space isn’t enough -- you must have at
least one index. Indexes make access faster.
Indexes can be declared to be "unique", which
is important because some combination of the
fields must be unique, for identification purposes.
More in the Bee manual:
http://bee.org/doc/master/user_guide.html#db.create_index

Request #17 is:

db.space.test:create_index("primary",{unique = true, parts = {1, "NUM"}})!
db.space.test:create_index("secondary",{parts = {2, "STR"}})!
--------------------------------------------------------------

This means the first index will be named primary,
will be unique, will be on the first field of each
tuple, and will be numeric. The second index will
be named secondary, doesn’t have to be unique, will
be on the second field of each tuple, and will be
in order by string value.
]];

[[
Tutorial -- Screen #18 -- Insert multiple tuples
================================================

In a loop, put some tuples in your new space.
Because of the index definitions, the first field
must be a number, the second field must be a string,
and the later fields can be anything.
Use a function in the Lua string library to make
values for the second field.
More in the Lua manual: http://www.lua.org/pil/20.html

Request #18 is:

for i=65,70,1 do
db.space.test:replace{i, string.char(i)}
end!
-------------------------------------------

Tip: to select the tuples later, use the function
that you created earlier: database_display("test")!
]];

[[
Tutorial -- Screen #19 -- Become another user
=============================================

Remember, you’re currently "admin" -- administrator.
Now switch to being "guest", a much less powerful user.

Request #19 is:
session.su("guest") -- switch user to "guest"!
db.space.test:replace{100,""} -- try to add a tuple!
-------------------------------

The result will be an error message telling you that
you don’t have the privilege to do that any more.
That’s good news. It shows that Bee prevents
unauthorized users from working with databases.
But you can say session.su("admin")! to become
a powerful user again, because for this tutorial
the "admin" user isn’t protected by a password.
]];

[[
Tutorial -- Screen #20 -- The Bigger Tutorials
==============================================

You can continue to type in whatever Lua instructions,
package requires, and database-manipulations you want,
here on this screen. But to really get into Bee,
you should download it so that you can be your own
administrator and create your own permanent databases. The
Bee manual has two significant tutorials:

Insert one million tuples with a Lua stored procedure
http://bee.org/doc/master/user_guide.html#lua-tutorial-insert
and
Sum a JSON field for all tuples
http://bee.org/doc/master/user_guide.html#lua-tutorial-sum

Request #20 is:

((Whatever you want. Enjoy!))

When you’re finished, don’t type <tutorial("next")!>, just wander off
and have a nice day.
]];
    }; --[[ tutorial ]]--
}
