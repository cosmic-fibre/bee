-------------------------------------------------------------------------------
                            Package `yaml`
-------------------------------------------------------------------------------

The ``yaml`` package takes strings in YAML_ format and decodes them, or takes a
series of non-YAML values and encodes them.

.. module:: yaml

.. function:: encode(lua_value)

    Convert a Lua object to a YAML string.

    :param lua_value: either a scalar value or a Lua table value.
    :return: the original value reformatted as a YAML string.
    :rtype: string

.. function:: decode(string)

    Convert a YAML string to a Lua object.

    :param string: a string formatted as YAML.
    :return: the original contents formatted as a Lua table.
    :rtype: table

.. _yaml-null:

.. data:: NULL

    A value comparable to Lua "nil" which may be useful as a placeholder in a tuple.

=================================================
                    Example
=================================================

.. code-block:: lua

    bee> yaml = require('yaml')
    ---
    ...
    bee> y =  yaml.encode({'a',1,'b',2})
    ---
    ...
    bee> z = yaml.decode(y)
    ---
    ...
    bee> z[1],z[2],z[3],z[4]
    ---
    - a
    - 1
    - b
    - 2
    ...
    bee> if yaml.NULL == nil then print('hi') end
    hi
    ---
    ...

.. _YAML: http://yaml.org/
