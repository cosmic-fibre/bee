.. _db-tuple:

-------------------------------------------------------------------------------
                            Package `db.tuple`
-------------------------------------------------------------------------------

.. module:: db.tuple

The ``db.tuple`` package provides read-only access for the ``db.tuple``
userdata type. It allows, for a single tuple: selective retrieval of the field
contents, retrieval of information about size, iteration over all the fields,
and conversion to a Lua table.

.. function:: new(value)

    Construct a new tuple from either a scalar or a Lua table. Alternatively,
    one can get new tuples from bee's SQL-like statements: SELECT,
    INSERT, UPDATE, REPLACE, which can be regarded as statements that do
    ``new()`` implicitly.

    :param lua-value value: the value that will become the tuple contents.

    :return: a new tuple
    :rtype:  tuple

    In the following example, ``x`` will be a new table object containing one
    tuple and ``t`` will be a new tuple object. Saying ``t`` returns the
    entire tuple ``t``.

    .. code-block:: lua

        bee> x = db.space.tester:insert{33,tonumber('1'),tonumber64('2')}:totable()
        ---
        ...
        bee> t = db.tuple.new({'abc', 'def', 'ghi', 'abc'})
        ---
        ...
        bee> t
        ---
        - ['abc', 'def', 'ghi', 'abc']
        ...

.. class:: tuple

    #tuple-value

        The ``#`` operator in Lua means "return count of components". So,
        if ``t`` is a tuple instance, ``#t`` will return the number of fields.

        :rtype: number

        In the following example, a tuple named ``t`` is created and then the
        number of fields in ``t`` is returned.

        .. code-block:: lua

            bee> t = db.tuple.new({'Fld#1','Fld#2','Fld#3','Fld#4'})
            ---
            ...
            bee> #t
            ---
            - 4
            ...

    tuple-value:bsize()

        If ``t`` is a tuple instance, ``t:bsize()`` will return the number of
        bytes in the tuple. It is useful to check this number when making
        changes to data, because there is a fixed maximum: one megabyte. Every
        field has one or more "length" bytes preceding the actual contents, so
        ``bsize()`` returns a value which is slightly greater than the sum of
        the lengths of the contents.

        :return: number of bytes
        :rtype: number

        In the following example, a tuple named ``t`` is created which has
        three fields, and for each field it takes one byte to store the length
        and three bytes to store the contents, and a bit for overhead, so
        ``bsize()`` returns ``3*(1+3)+1``.

        .. code-block:: lua

            bee> t = db.tuple.new({'aaa','bbb','ccc'})
            ---
            ...
            bee> t:bsize()
            ---
            - 13
            ...

    tuple-value [field-number]

        If ``t`` is a tuple instance, ``t[field-number]`` will return the field
        numbered field-number in the tuple. The first field is ``t[1]``.

        :return: field value.
        :rtype:  lua-value

        In the following example, a tuple named ``t`` is created and then the
        second field in ``t`` is returned.

        .. code-block:: lua

            bee> t = db.tuple.new({'Fld#1','Fld#2','Fld#3','Fld#4'})
            ---
            ...
            bee> t[2]
            ---
            - Fld#2
            ...

    tuple-value:find([field-number,] search-value) or tuple-value:findall([field-number,] search-value)

        If ``t`` is a tuple instance, ``t:find(search-value)`` will return the
        number of the first field in ``t`` that matches the search value),
        and ``t:findall(search-value [, search-value ...])`` will return numbers
        of all fields in ``t`` that match the search value. Optionally one can
        put a numeric argument ``field-number`` before the search-value to
        indicate “start searching at field number ``field-number``.”

        :return: the number of the field in the tuple.
        :rtype:  number

        In the following example, a tuple named ``t`` is created and then: the
        number of the first field in ``t`` which matches 'a' is returned, then
        the numbers of all the fields in ``t`` which match 'a' are returned,
        then the numbers of all the fields in t which match 'a' and are at or
        after the second field are returned.

        .. code-block:: lua

            bee> t = db.tuple.new({'a','b','c','a'})
            ---
            ...
            bee> t:find('a')
            ---
            - 1
            ...
            bee> t:findall('a')
            ---
            - 1
            - 4
            ...
            bee> t:findall(2, 'a')
            ---
            - 4
            ...

    tuple-value:transform(start-field-number, fields-to-remove [, field-value ...])

        If ``t`` is a tuple instance, ``t:transform(start-field-number,fields-to-remove)``
        will return a tuple where, starting from field ``start-field-number``,
        a number of fields (``fields-to-remove``) are removed. Optionally one
        can add more arguments after ``fields-to-remove`` to indicate new
        values that will replace what was removed.

        :param integer start-field-number: base 1, may be negative
        :param integer   fields-to-remove:
        :param lua-value   field-value(s):
        :return: tuple
        :rtype:  tuple

        In the following example, a tuple named ``t`` is created and then,
        starting from the second field, two fields are removed but one new
        one is added, then the result is returned.

        .. code-block:: lua

            bee> t = db.tuple.new({'Fld#1','Fld#2','Fld#3','Fld#4','Fld#5'})
            ---
            ...
            bee> t:transform(2,2,'x')
            ---
            - ['Fld#1', 'x', 'Fld#4', 'Fld#5']
            ...

    tuple-value:unpack([start-field-number [, end-field-number]])

        If ``t`` is a tuple instance, ``t:unpack()`` will return all fields,
        ``t:unpack(1)`` will return all fields starting with field number 1,
        ``t:unpack(1,5)`` will return all fields between field number 1 and field number 5.

        :return: field(s) from the tuple.
        :rtype:  lua-value(s)

        In the following example, a tuple named ``t`` is created and then all
        its fields are selected, then the result is returned.

        .. code-block:: lua

            bee> t = db.tuple.new({'Fld#1','Fld#2','Fld#3','Fld#4','Fld#5'})
            ---
            ...
            bee> t:unpack()
            ---
            - Fld#1
            - Fld#2
            - Fld#3
            - Fld#4
            - Fld#5
            ...

    tuple-value:pairs()

        In Lua, ``lua-table-value:pairs()`` is a method which returns:
        ``function``, ``lua-table-value``, ``nil``. Bee has extended
        this so that ``tuple-value:pairs()`` returns: ``function``,
        ``tuple-value``, ``nil``. It is useful for Lua iterators, because Lua
        iterators traverse a value's components until an end marker is reached.

        :return: function, tuple-value, nil
        :rtype:  function, lua-value, nil

        In the following example, a tuple named ``t`` is created and then all
        its fields are selected using a Lua for-end loop.

        .. code-block:: lua

            bee> t = db.tuple.new({'Fld#1','Fld#2','Fld#3','Fld#4','Fld#5'})
            ---
            ...
            bee> tmp = ''; for k, v in t:pairs() do tmp = tmp .. v end
            ---
            ...
            bee> tmp
            ---
            - Fld#1Fld#2Fld#3Fld#4Fld#5
            ...

    tuple-value:update({{format, field_number, value}...})

        Update a tuple.

        This function updates a tuple which is not in a space. Compare the function
        ``db.space.space-name:update{key, format, {field_number, value}...)``,
        which updates a tuple in a space.

        Parameters: briefly: format indicates the type of update operation such as '``=``'
        for 'assign new value', ``field_number`` indicates the field number to change such
        as 2 for field number 2, value indicates the string which operates on the field such
        as 'B' for a new assignable value = 'B'.

        For details: see the description for ``format``, ``field_number``, and ``value`` in
        the section ``db.space.space-name:update{key, format, {field_number, value}...)``.

        :return: new tuple
        :rtype:  tuple

        In the following example, a tuple named ``t`` is created and then its second field is
        updated to equal 'B'.

        .. code-block:: lua

            bee> t = db.tuple.new({'Fld#1','Fld#2','Fld#3','Fld#4','Fld#5'})
            ---
            ...
            bee> t:update({{'=',2,'B'}})
            ---
            - ['Fld#1', 'B', 'Fld#3', 'Fld#4', 'Fld#5']
            ...

===========================================================
                        Example
===========================================================

This function will illustrate how to convert tuples to/from Lua tables and
lists of scalars:

.. code-block:: lua

    tuple = db.tuple.new({scalar1, scalar2, ... scalar_n}) -- scalars to tuple
    lua_table = {tuple:unpack()}                            -- tuple to Lua table
    scalar1, scalar2, ... scalar_n = tuple:unpack()         -- tuple to scalars
    tuple = db.tuple.new(lua_table)                        -- Lua table to tuple

Then it will find the field that contains 'b', remove that field from the tuple,
and display how many bytes remain in the tuple. The function uses Bee
``db.tuple`` functions ``new()``, ``unpack()``, ``find()``, ``transform()``,
``bsize()``.

.. code-block:: lua

    console = require('console'); console.delimiter('!')
    function example()
      local tuple1, tuple2, lua_table_1, scalar1, scalar2, scalar3, field_number
      tuple1 = db.tuple.new({'a', 'b', 'c'})
      luatable1 = {tuple1:unpack()}
      scalar1, scalar2, scalar3 = tuple1:unpack()
      tuple2 = db.tuple.new(luatable1)
      field_number = tuple2:find('b')
      tuple2 = tuple2:transform(field_number, 1)
      return 'tuple2 = ' , tuple2 , ' # of bytes = ' , tuple2:bsize()
    end!
    console.delimiter('')!

... And here is what happens when one invokes the function:

.. code-block:: yaml

    bee> example()
    ---
    - 'tuple2 = '
    - ['a', 'c']
    - ' # of bytes = '
    - 5
    ...
