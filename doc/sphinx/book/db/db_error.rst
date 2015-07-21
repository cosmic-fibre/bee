-------------------------------------------------------------------------------
                            Package `db.error`
-------------------------------------------------------------------------------

The ``db.error`` function is for raising an error. The difference between this
function and Lua's built-in ``error()`` function is that when the error reaches
the client, its error code is preserved. In contrast, a Lua error would always
be presented to the client as ``ER_PROC_LUA``.

.. module:: db.error

.. function:: db.error{reason=string [, code=number]}

    When called with a Lua-table argument, the code and reason have any
    user-desired values. The result will be those values.

    :param integer  code:
    :param string reason:

.. function:: db.error()

    When called without arguments, ``db.error()`` re-throws whatever the last
    error was.

.. function:: db.error(code, errtext [, errtext ...])

    Emulate a request error, with text based on one of the pre-defined Bee
    errors defined in the file `errcode.h
    <https://github.com/bee/bee/blob/master/src/db/errcode.h>`_ in
    the source tree. Lua constants which correspond to those Bee errors are
    defined as members of ``db.error``, for example ``db.error.NO_SUCH_USER == 45``.

    :param number       code: number of a pre-defined error
    :param string errtext(s): part of the message which will accompany the error

    For example:

    the ``NO_SUCH_USER`` message is "``User '%s' is not found``" -- it includes
    one "``%s``" component which will be replaced with errtext. Thus a call to
    ``db.error(db.error.NO_SUCH_USER, 'joe')`` or ``db.error(45, 'joe')``
    will result in an error with the accompanying message
    "``User 'joe' is not found``".

    :except: whatever is specified in errcode-number.

    .. code-block:: lua

        bee> db.error({code=555, reason='Arbitrary message'})
        ---
        - error: Arbitrary message
        ...
        bee> db.error()
        ---
        - error: Arbitrary message
        ...
        bee> db.error(db.error.FUNCTION_ACCESS_DENIED, 'A', 'B', 'C')
        ---
        - error: A access denied for user 'B' to function 'C'
        ...

.. function:: db.error.last()

    Returns a description of the last error, as a Lua table
    with three members: "type" (string) error's C++ class,
    "message" (string) error's message, "code" (numeric) error's number.
    Additionally, if the error is a system error (for example due to a
    failure in socket or file io), there is a fourth member:
    "errno" (number) C standard error number.

    rtype: table

.. function:: db.error.clear()

    Clears the record of errors, so functions like `db.error()`
    or `db.error.last()` will have no effect.

    .. code-block:: lua

        EXAMPLE

        bee> db.error({code=555, reason='Arbitrary message'})
        ---
        - error: Arbitrary message
        ...

        bee> db.error.last()
        ---
        - type: ClientError
          message: Arbitrary message
          code: 555
        ...

        bee> db.error.clear()
        ---
        ...

        bee> db.error.last()
        ---
        - null
        ...
