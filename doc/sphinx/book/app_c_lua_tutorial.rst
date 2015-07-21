-------------------------------------------------------------------------------
                        Appendix C. Lua tutorial
-------------------------------------------------------------------------------

=====================================================================
       Insert one million tuples with a Lua stored procedure
=====================================================================

This is an exercise assignment: “Insert one million tuples. Each tuple should
have a constantly-increasing numeric primary-key field and a random alphabetic
10-character string field.”

The purpose of the exercise is to show what Lua functions look like inside
Bee. It will be necessary to employ the Lua math library, the Lua string
library, the Bee db library, the Bee db.tuple library, loops, and
concatenations. It should be easy to follow even for a person who has not used
either Lua or Bee before. The only requirement is a knowledge of how other
programming languages work and a memory of the first two chapters of this manual.
But for better understanding, follow the comments and the links, which point to
the Lua manual or to elsewhere in this Bee manual. To further enhance
learning, type the statements in with the bee client while reading along.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        Configure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We are going to use the "bee_sanddb" that was created in section
:ref:`first database`. So there is a single space, and a numeric primary key,
and a running bee server which also serves as a client.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        Delimiter
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We'll be making functions which go over one line. We don't want the client to
send to the server after every line. So we :ref:`declare a delimiter <setting delimiter>`.
This means “Do not send to the server until you see an exclamation mark.”

.. code-block:: lua

    bee> console = require('console'); console.delimiter('!')

From now on it will be possible to use multiple-line statements, but it will be
necessary to end all statements with exclamation marks.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
           Create a function that returns string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We will start by making a function that returns a fixed string, “Hello world”.

.. code-block:: lua

    function string_function()
      return "hello world"
    end!

The word "``function``" is a Lua keyword -- we're about to go into Lua. The
function name is string_function. The function has one executable statement,
``return "hello world"``. The string "hello world" is enclosed in double quotes
here, although Lua doesn't care -- one could use single quotes instead. The
word "``end``" means “this is the end of the Lua function declaration.” The
word "``end``" is followed by "``!``" because "``!``" happens to be the
delimiter that we chose in the previous step. To confirm that the function works,
we can say

.. code-block:: lua

    string_function()!

Sending ``function-name()`` means “invoke the Lua function.” The effect is
that the string which the function returns will end up on the screen.

For more about Lua strings see Lua manual `chapter 2.4 "Strings"`_ . For more
about functions see Lua manual `chapter 5 "Functions"`_.

.. _chapter 2.4 "Strings": http://www.lua.org/pil/2.4.html
.. _chapter 5 "Functions": http://www.lua.org/pil/5.html

The screen now looks like this:

.. code-block:: lua

    bee> function string_function()
            ->   return "hello world"
            -> end!
    ---
    ...
    bee> string_function()!
    ---
    - hello world
    ...
    bee>

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Create a function that calls another function and sets a variable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that ``string_function`` exists, we can invoke it from another
function.

.. code-block:: lua

    function main_function()
      local string_value
      string_value = string_function()
      return string_value
    end!

We begin by declaring a variable "``string_value``". The word "``local``"
means that string_value appears only in ``main_function``. If we didn't use
"``local``" then ``string_value`` would be visible everywhere - even by other
users using other clients connected to this server! Sometimes that's a very
desirable feature for inter-client communication, but not this time.

Then we assign a value to ``string_value``, namely, the result of
``string_function()``. Soon we will invoke ``main_function()`` to check that it
got the value.

For more about Lua variables see Lua manual `chapter 4.2 "Local Variables and Blocks"`_ .

.. _chapter 4.2 "Local Variables and Blocks": http://www.lua.org/pil/4.2.html

The screen now looks like this:

.. code-block:: lua

    bee> function main_function()
            ->   local string_value
            ->   string_value = string_function()
            ->   return string_value
            -> end!
    ---
    ...
    bee> main_function()!
    ---
    - hello world
    ...
    bee>

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Modify the function so it returns a one-letter random string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that it's a bit clearer how to make a variable, we can change
``string_function()`` so that, instead of returning a fixed literal
'Hello world", it returns a random letter between 'A' and 'Z'.

.. code-block:: lua

    function string_function()
      local random_number
      local random_string
      random_number = math.random(65, 90)
      random_string = string.char(random_number)
      return random_string
    end!

It is not necessary to destroy the old ``string_function()`` contents, they're
simply overwritten. The first assignment invokes a random-number function
in Lua's math library; the parameters mean “the number must be an integer
between 65 and 90.” The second assignment invokes an integer-to-character
function in Lua's string library; the parameter is the code point of the
character. Luckily the ASCII value of 'A' is 65 and the ASCII value of 'Z'
is 90 so the result will always be a letter between A and Z.

For more about Lua math-library functions see Lua users "`Math Library Tutorial`_".
For more about Lua string-library functions see Lua users "`String Library Tutorial`_" .

.. _Math Library Tutorial: http://lua-users.org/wiki/MathLibraryTutorial
.. _String Library Tutorial: http://lua-users.org/wiki/StringLibraryTutorial

Once again the ``string_function()`` can be invoked from main_function() which
can be invoked with ``main_function()!``

The screen now looks like this:

.. code-block:: lua

    bee> function string_function()
            ->   local random_number
            ->   local random_string
            ->   random_number = math.random(65, 90)
            ->   random_string = string.char(random_number)
            ->   return random_string
            -> end!
    ---
    ...
    bee> main_function()!
    ---
    - C
    ...
    bee>

... Well, actually it won't always look like this because ``math.random()``
produces random numbers. But for the illustration purposes it won't matter
what the random string values are.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Modify the function so it returns a ten-letter random string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that it's clear how to produce one-letter random strings, we can reach our
goal of producing a ten-letter string by concatenating ten one-letter strings,
in a loop.

.. code-block:: lua

    function string_function()
      local random_number
      local random_string
      random_string = ""
      for x = 1,10,1 do
        random_number = math.random(65, 90)
        random_string = random_string .. string.char(random_number)
      end
      return random_string
    end!

The words "for x = 1,10,1" mean “start with x equals 1, loop until x equals 10,
increment x by 1 for each iteration.” The symbol ".." means "concatenate", that
is, add the string on the right of the ".." sign to the string on the left of
the ".." sign. Since we start by saying that random_string is "" (a blank
string), the end result is that random_string has 10 random letters. Once
again the ``string_function()`` can be invoked from ``main_function()`` which
can be invoked with ``main_function()!``

For more about Lua loops see Lua manual `chapter 4.3.4 "Numeric for"`_.

.. _chapter 4.3.4 "Numeric for": http://www.lua.org/pil/4.3.4.html

The screen now looks like this:

.. code-block:: lua

    bee> function string_function()
            ->   local random_number
            ->   local random_string
            ->   random_string = ""
            ->   for x = 1,10,1 do
            ->     random_number = math.random(65, 90)
            ->     random_string = random_string .. string.char(random_number)
            ->   end
            ->   return random_string
            -> end!
    ---
    ...
    bee> main_function()!
    ---
    - 'ZUDJBHKEFM'
    ...
    bee>


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
           Make a tuple out of a number and a string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that it's clear how to make a 10-letter random string, it's possible to
make a tuple that contains a number and a 10-letter random string, by invoking
a function in Bee's library of Lua functions.

.. code-block:: lua

    function main_function()
      local string_value
      string_value = string_function()
      t = db.tuple.new({1, string_value})
      return t
    end!

Once this is done, t will be the value of a new tuple which has two fields.
The first field is numeric: 1. The second field is a random string. Once again
the ``string_function()`` can be invoked from ``main_function()`` which can be
invoked with ``main_function()!``

For more about Bee tuples see Bee manual section Package :mod:`db.tuple`.

The screen now looks like this:

.. code-block:: lua

    bee> function main_function()
            ->   local string_value
            ->   string_value = string_function()
            ->   t = db.tuple.new({1, string_value})
            ->   return t
            -> end!
    ---
    ...
    bee> main_function()!
    ---
    - [1, 'PNPZPCOOKA']
    ...
    bee>

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     Modify main_function to insert a tuple into the database
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that it's clear how to make a tuple that contains a number and a 10-letter
random string, the only trick remaining is putting that tuple into tester.
Remember that tester is the first space that was defined in the sanddb, so
it's like a database table.

.. code-block:: lua

    function main_function()
      local string_value
      string_value = string_function()
      t = db.tuple.new({1,string_value})
      db.space.tester:replace(t)
    end!

The new line here is ``db.space.tester:replace(t)``. The name contains
'tester' because the insertion is going to be to tester. The second parameter
is the tuple value. To be perfectly correct we could have said
``db.space.tester:insert(t)`` here, rather than ``db.space.tester:replace(t)``,
but "replace" means “insert even if there is already a tuple whose primary-key
value is a duplicate”, and that makes it easier to re-run the exercise even if
the sanddb database isn't empty. Once this is done, tester will contain a tuple
with two fields. The first field will be 1. The second field will be a random
10-letter string. Once again the ``string_function(``) can be invoked from
``main_function()`` which can be invoked with ``main_function()!``. But
``main_function()`` won't tell the whole story, because it does not return t, it
nly puts t into the database. To confirm that something got inserted, we'll use
a SELECT request.

.. code-block:: lua

    main_function()!
    db.space.tester:select{1}!

For more about Bee insert and replace calls, see Bee manual section
:mod:`db.space`

The screen now looks like this:

.. code-block:: lua

    bee> function main_function()
            ->   local string_value
            ->   string_value = string_function()
            ->   t = db.tuple.new({1,string_value})
            ->   db.space.tester:replace(t)
            -> end!
    ---
    ...
    bee> main_function()!
    ---
    ...
    bee> db.space.tester:select{1}!
    ---
    - - [1, 'EUJYVEECIL']
    ...
    bee>

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Modify main_function to insert a million tuples into the database
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now that it's clear how to insert one tuple into the database, it's no big deal
to figure out how to scale up: instead of inserting with a literal value = 1
for the primary key, insert with a variable value = between 1 and 1 million, in
a loop. Since we already saw how to loop, that's a simple thing. The only extra
wrinkle that we add here is a timing function.

.. code-block:: lua

    function main_function()
      local string_value
      start_time = os.clock()
      for i = 1,1000000,1 do
        string_value = string_function()
        t = db.tuple.new({i,string_value})
        db.space.tester:replace(t)
      end
      end_time = os.clock()
    end!
    main_function()!
    'insert done in ' .. end_time - start_time .. ' seconds'!

The Lua ``os.clock()`` function will return the number of seconds since the
start. Therefore, by getting start_time = number of seconds just before the
inserting, and then getting end_time = number of seconds just after the
inserting, we can calculate (end_time - start_time) = elapsed time in seconds.
We will display that value by putting it in a request without any assignments,
which causes Bee to send the value to the client, which prints it. (Lua's
answer to the C ``printf()`` function, which is ``print()``, will also work.)

For more on Lua ``os.clock()`` see Lua manual `chapter 22.1 "Date and Time"`_ . For more on Lua print() see Lua manual `chapter 5 "Functions"`_.

.. _chapter 22.1 "Date and Time": http://www.lua.org/pil/22.1.html
.. _chapter 5 "Functions": http://www.lua.org/pil/5.html

Since this is the grand finale, we will redo the final versions of all the
necessary requests: the ``console.delimiter('!')`` request, the request that
created ``string_function()``, the request that created ``main_function()``,
and the request that invokes ``main_function()``.

.. code-block:: lua

    -- Skip the following statement if you have already said "console.delimiter('!')"
    console = require('console'); console.delimiter('!')

    function string_function()
      local random_number
      local random_string
      random_string = ""
      for x = 1,10,1 do
        random_number = math.random(65, 90)
        random_string = random_string .. string.char(random_number)
      end
      return random_string
    end!

    function main_function()
      local string_value
      start_time = os.clock()
      for i = 1,1000000,1 do
        string_value = string_function()
        t = db.tuple.new({i,string_value})
        db.space.tester:replace(t)
      end
      end_time = os.clock()
    end!
    main_function()!
    'insert done in ' .. end_time - start_time .. ' seconds'!

The screen now looks like this:

.. code-block:: lua

    bee> console = require('console'); console.delimiter('!')
    bee> function string_function()
            ->   local random_number
            ->   local random_string
            ->   random_string = ""
            ->   for x = 1,10,1 do
            ->     random_number = math.random(65, 90)
            ->     random_string = random_string .. string.char(random_number)
            ->   end
            ->   return random_string
            -> end!
    ---
    ...
    bee> function main_function()
            ->   local string_value
            ->   start_time = os.clock()
            ->   for i = 1,1000000,1 do
            ->     string_value = string_function()
            ->     t = db.tuple.new({i,string_value})
            ->     db.space.tester:replace(t)
            ->   end
            ->   end_time = os.clock()
            -> end!
    ---
    ...
    bee> main_function()!
    ---
    ...
    bee> 'insert done in ' .. end_time - start_time .. ' seconds'!
    ---
    - insert done in 60.62 seconds
    ...
    bee>

What has been shown is that Lua functions are quite expressive (in fact one can
do more with Bee's Lua stored procedures than one can do with stored
procedures in some SQL DBMSs), and that it's straightforward to combine
Lua-library functions and Bee-library functions.

What has also been shown is that inserting a million tuples took 60 seconds. The
host computer was a Toshiba laptop with a 2.2-GHz Intel Core Duo CPU.


=====================================================================
                  Sum a JSON field for all tuples
=====================================================================

This is an exercise assignment: “Assume that inside every tuple there is a
string formatted as JSON. Inside that string there is a JSON numeric field.
For each tuple, find the numeric field's value and add it to a 'sum' variable.
At end, return the 'sum' variable.” The purpose of the exercise is to get
experience in one way to read and process tuples.

.. code-block:: lua

    console = require('console'); console.delimiter('!')
    function sum_json_field(field_name)
      json = require('json')
      local v, t, sum, field_value, is_valid_json, lua_table                --[[1]]
      sum = 0                                                               --[[2]]
      for v, t in db.space.tester:pairs() do                               --[[3]]
        is_valid_json, lua_table = pcall(json.decode, t[2])                 --[[4]]
        if is_valid_json then                                               --[[5]]
          field_value = lua_table[field_name]                               --[[6]]
          if type(field_value) == "number" then sum = sum + field_value end --[[7]]
        end                                                                 --[[8]]
      end                                                                   --[[9]]
      return sum                                                            --[[10]]
    end!
    console.delimiter('')!

LINE 1: WHY "LOCAL". This line declares all the variables that will be used in
the function. Actually it's not necessary to declare all variables at the start,
and in a long function it would be better to declare variables just before using
them. In fact it's not even necessary to declare variables at all, but an
undeclared variable is "global". That's not desirable for any of the variables
that are declared in line 1, because all of them are for use only within the function.

LINE 3: WHY "PAIRS()". Our job is to go through all the rows and there are two
ways to do it: with ``db.space.space-name:pairs()`` or with
:func:`index.iterator <index_object.pairs>`.
We preferred ``pairs()`` because it is simpler.

LINE 4: WHY "PCALL". If we simply said "``lua_table = json.decode(t[2]))``", then
the function would abort with an error if it encountered something wrong with the
JSON string - a missing colon, for example. By putting the function inside "``pcall``"
(`protected call`_), we're saying: we want to intercept that sort of error, so if
there's a problem just set ``is_valid_json = false`` and we will know what to do
about it later.

LINE 4: MEANING. The function is :func:`json.decode` which means decode a JSON
string, and the parameter is t[2] which is a reference to a JSON string. There's
a bit of hard coding here, we're assuming that the second field in the tuple is
where the JSON string was inserted. For example, we're assuming a tuple looks like

.. _protected call: http://www.lua.org/pil/8.4.html

.. code-block:: json

    field[1]: 444
    field[2]: '{"Hello": "world", "Quantity": 15}'

meaning that the tuple's first field, the primary key field, is a number while
the tuple's second field, the JSON string, is a string. Thus the entire statement
means "decode ``t[2]`` (the tuple's second field) as a JSON string; if there's an
error set ``is_valid_json = false``; if there's no error set ``is_valid_json = true`` and
set ``lua_table =`` a Lua table which has the decoded string".

LINE 6. At last we are ready to get the JSON field value from the Lua table that
came from the JSON string. The value in field_name, which is the parameter for the
whole function, must be a name of a JSON field. For example, inside the JSON string
``'{"Hello": "world", "Quantity": 15}'``, there are two JSON fields: "Hello" and
"Quantity". If the whole function is invoked with ``sum_json_field("Quantity")``,
then ``field_value = lua_table[field_name]`` is effectively the same as
``field_value = lua_table["Quantity"]`` or even ``field_value = lua_table.Quantity``.
Those are just three different ways of saying: for the Quantity field in the Lua table,
get the value and put it in variable field_value.

LINE 7: WHY "IF". Suppose that the JSON string is well formed but the JSON field
is not a number, or is missing. In that case, the function would be aborted when
there was an attempt to add it to the sum. By first checking
``type(field_value) == "number"``, we avoid that abortion. Anyone who knows that
the database is in perfect shape can skip this kind of thing.

And the function is complete. Time to test it. Starting with an empty database,
defined the same way as the sanddb database that was introduced in
“ :ref:`first database` ”,

.. code-block:: lua

    -- if tester is left over from some previous test, destroy it
    db.space.tester:drop()
    db.schema.space.create('tester')
    db.space.tester:create_index('primary', {parts = {1, 'NUM'}})

then add some tuples where the first field is a number and the second
field is a string.

.. code-block:: lua

    db.space.tester:insert{444, '{"Item": "widget", "Quantity": 15}'}
    db.space.tester:insert{445, '{"Item": "widget", "Quantity": 7}'}
    db.space.tester:insert{446, '{"Item": "golf club", "Quantity": "sunshine"}'}
    db.space.tester:insert{447, '{"Item": "waffle iron", "Quantit": 3}'}

Since this is a test, there are deliberate errors. The "golf club" and the
"waffle iron" do not have numeric Quantity fields, so must be ignored.
Therefore the real sum of the Quantity field in the JSON strings should be:
15 + 7 = 22.

Invoke the function with ``sum_json_field("Quantity")``.

.. code-block:: lua

    bee> sum_json_field("Quantity")
    ---
    - 22
    ...

It works. We'll just leave, as exercises for future improvement, the possibility
that the "hard coding" assumptions could be removed, that there might have to be
an overflow check if some field values are huge, and that the function should
contain a "yield" instruction if the count of tuples is huge.











