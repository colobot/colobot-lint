#!/usr/bin/env python3
import unittest

loader = unittest.TestLoader()
testmodules = loader.discover('.', pattern='*_test.py')

# .py modules
for testmodule in testmodules:
    # TestSuite classes in each module
    for testsuite in testmodule._tests:
        # testcase functions in each TestSuite
        for test in testsuite._tests:
            testname = test.__str__().split()
            print(testname[1][1:-1] + "." + testname[0])