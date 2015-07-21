import os
import re
import sys
import glob
import traceback
import shutil
from subprocess import Popen, PIPE

from lib.server import Server
from lib.bee_server import Test

class AppTest(Test):
    def execute(self, server):
        execs = []
        proc = Popen([os.path.join(os.getcwd(), self.name)], stdout=PIPE, cwd=server.vardir)
        sys.stdout.write(proc.communicate()[0])

class AppServer(Server):
    """A dummy server implementation for application server tests"""
    def __new__(cls, ini=None):
        return Server.__new__(cls)

    def __init__(self, _ini=None):
        if _ini is None:
            _ini = {}
        ini = {
            'vardir': None
        }; ini.update(_ini)
        Server.__init__(self, ini)
        self.testdir = os.path.abspath(os.curdir)
        self.vardir = ini['vardir']
        self.re_vardir_cleanup += [
            "*.snap", "*.xlog", "*.inprogress",
            "*.sup", "*.lua", "*.pid"]
        self.cleanup()
        self.builddir = ini['builddir']
        self.debug = False
        self.lua_libs = ini['lua_libs']

    def deploy(self, vardir=None, silent=True, need_init=True):
        self.vardir = vardir
        if not os.access(self.vardir, os.F_OK):
            os.makedirs(self.vardir)
        if self.lua_libs:
            for i in self.lua_libs:
                source = os.path.join(self.testdir, i)
                shutil.copy(source, self.vardir)

    @classmethod
    def find_exe(cls, builddir):
        cls.builddir = builddir

    def find_tests(self, test_suite, suite_path):
        def patterned(test, patterns):
            answer = []
            for i in patterns:
                if test.name.find(i) != -1:
                    answer.append(test)
            return answer

        test_suite.tests = [AppTest(k, test_suite.args, test_suite.ini) for k in sorted(glob.glob(os.path.join(suite_path, "*.test.lua" )))]
        test_suite.tests = sum(map((lambda x: patterned(x, test_suite.args.tests)), test_suite.tests), [])

    def print_log(self, lines):
        pass
