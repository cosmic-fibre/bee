#!/usr/bin/env python2
"""Bee regression test suite front-end."""

__author__ = "Konstantin Osipov <kostja.osipov@gmail.com>"

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

import os
import sys
import time
import string
import shutil
import os.path
import argparse

from lib.colorer          import Colorer
from lib.parallel         import Supervisor
from lib.test_suite       import TestSuite
from lib.bee_server import BeeServer
from lib.unittest_server  import UnittestServer
color_stdout = Colorer()
#
# Run a collection of tests.
#

class Options:
    """Handle options of test-runner"""
    def __init__(self):
        """Add all program options, with their defaults."""

        parser = argparse.ArgumentParser(
                description = "Bee regression test suite front-end.")

        parser.epilog = "For a complete description, use 'pydoc ./" +\
                os.path.basename(sys.argv[0]) + "'"

        parser.add_argument(
                "tests",
                metavar="test",
                nargs="*",
                default = [""],
                help="""Can be empty. List of test names, to look for in suites. Each
                name is used as a substring to look for in the path to test file,
                e.g. "show" will run all tests that have "show" in their name in all
                suites, "db/show" will only enable tests starting with "show" in
                "db" suite. Default: run all tests in all specified suites.""")

        parser.add_argument(
                "--suite",
                dest = 'suites',
                metavar = "suite",
                nargs="*",
                default = [],
                help = """List of test suites to look for tests in. Default: "" -
                means find all available.""")

        parser.add_argument(
                "--force",
                dest = "is_force",
                action = "store_true",
                default = False,
                help = """Go on with other tests in case of an individual test failure.
                Default: false.""")

        parser.add_argument(
                "--gdb",
                dest = "gdb",
                action = "store_true",
                default = False,
                help = """Start the server under 'gdb' debugger in detached
                Screen. This option is mutually exclusive with --valgrind.
                Default: false.""")

        parser.add_argument(
                "--valgrind",
                dest = "valgrind",
                action = "store_true",
                default = False,
                help = "Run the server under 'valgrind'. Default: false.")

        parser.add_argument(
                "--builddir",
                dest = "builddir",
                default = "..",
                help = """Path to project build directory. Default: " + "../.""")

        parser.add_argument(
                "--stress",
                dest = "stress",
                default = None,
                help = """Name of streess TestSuite to run""")

        parser.add_argument(
                "--bee-port",
                dest = "bee_port",
                default = None,
                help = """Listen port number to run tests against. Admin port number must be listen+1""")

        parser.add_argument(
                "--vardir",
                dest = "vardir",
                default = "var",
                help = """Path to data directory. Default: var.""")
        parser.add_argument(
               "--long",
               dest="long",
               default=False,
               action='store_true',
               help="""Enable long run tests""")

        self.args = parser.parse_args()
        self.check()

    def check(self):
        """Check the arguments for correctness."""
        check_error = False
        if self.args.gdb and self.args.valgrind:
            color_stdout("Error: option --gdb is not compatible with option --valgrind", schema='error')
            check_error = True
        if check_error:
            exit(-1)


def setenv():
    """Find where is bee dir by check_file"""
    check_file = 'src/fiber.h'
    path = os.path.abspath('../')
    while path != '/':
        if os.path.isfile('%s/%s' % (path, check_file)):
            os.putenv('BEE_SRC_DIR', path)
            break
        path = os.path.abspath(os.path.join(path, '../'))

#######################################################################
# Program body
#######################################################################

def main():
    options = Options()
    oldcwd = os.getcwd()
    # Change the current working directory to where all test
    # collections are supposed to reside
    # If script executed with (python test-run.py) dirname is ''
    # so we need to make it .
    path = os.path.dirname(sys.argv[0])
    if not path:
        path = '.'
    os.chdir(path)
    setenv()

    failed_tests = []

    try:
        BeeServer.find_exe(options.args.builddir)
        UnittestServer.find_exe(options.args.builddir)

        color_stdout("Started {0}\n".format(" ".join(sys.argv)), schema='tr_text')
        suite_names = options.args.suites
        if suite_names == []:
            for root, dirs, names in os.walk(os.getcwd()):
                if "suite.ini" in names:
                    suite_names.append(os.path.basename(root))

        if options.args.stress is None:
            suites = [TestSuite(suite_name, options.args) for suite_name in sorted(suite_names)]
            for suite in suites:
                failed_tests.extend(suite.run_all())
        else:
            suite_names = [suite_name for suite_name in suite_names if suite_name.find(options.args.stress) != -1]
            suites = [Supervisor(suite_name, options.args) for suite_name in sorted(suite_names)]
            for suite in suites:
                suite.run_all()
    except RuntimeError as e:
        color_stdout("\nFatal error: %s. Execution aborted.\n" % e, schema='error')
        if options.args.gdb:
            time.sleep(100)
        return (-1)
    finally:
        os.chdir(oldcwd)

    if failed_tests and options.args.is_force:
        color_stdout("\n===== %d tests failed:\n" % len(failed_tests), schema='error')
        for test in failed_tests:
             color_stdout("----- %s\n" % test, schema='info')

    return (-1 if failed_tests else 0)

if __name__ == "__main__":
    exit(main())
