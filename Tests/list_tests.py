#!/usr/bin/env python3
import unittest
import os

def list_tests():
    tests = []

    loader = unittest.TestLoader()
    this_dir = os.path.dirname(os.path.realpath(__file__))
    testmodules = loader.discover(this_dir, pattern='*_test.py')

    # .py modules
    for testmodule in testmodules:
        # TestSuite classes in each module
        for testsuite in testmodule._tests:
            # testcase functions in each TestSuite
            for test in testsuite._tests:
                testname = test.__str__().split()
                tests.append(testname[1][1:-1] + "." + testname[0])

    return tests

if __name__ == '__main__':
    for test in list_tests():
        print(test)