#!/usr/bin/env python3
import argparse
import subprocess
import os
import sys
import list_tests
import fnmatch

parser = argparse.ArgumentParser(add_help = False)
parser.add_argument('--colobot-lint-exec', dest='colobot_lint_executable', required=True)
parser.add_argument('--debug', dest='debug_flag', action='store_true')
parser.add_argument('--filter', dest='filter')
options, remaining_args = parser.parse_known_args()

python_interp = sys.executable
this_dir = os.path.dirname(os.path.realpath(__file__))

if options.filter:
    filtered_tests = []
    for test in list_tests.list_tests():
        if fnmatch.fnmatch(test, options.filter):
            filtered_tests.append(test)

    if len(filtered_tests) == 0:
        sys.stderr.write("No testcases found matching filter '%s'\n" % options.filter)
        sys.exit(1)

    remaining_args.extend(filtered_tests)

if len(remaining_args) == 0:
    remaining_args = ['discover', '-s', this_dir, '-t', this_dir, '-p', '*_test.py']

code = subprocess.call([python_interp, '-m', 'unittest'] + remaining_args,
                       cwd = this_dir,
                       env = { 'COLOBOT_LINT': os.path.realpath(options.colobot_lint_executable),
                               'DEBUG': '1' if options.debug_flag else '0' })

sys.exit(code)