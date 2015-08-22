#!/usr/bin/env python3
import argparse
import subprocess
import os
import sys

parser = argparse.ArgumentParser(add_help = False)
parser.add_argument('--colobot-lint-exec', dest='colobot_lint_executable', required=True)
parser.add_argument('--debug', dest='debug_flag', action='store_true')
options, remaining_args = parser.parse_known_args()

python_interp = sys.executable
this_dir = os.path.dirname(os.path.realpath(__file__))

if len(remaining_args) == 0:
    remaining_args = ['discover', '-s', this_dir, '-t', this_dir, '-p', '*_test.py']

subprocess.call([python_interp, '-m', 'unittest'] + remaining_args,
                cwd = this_dir,
                env = { 'COLOBOT_LINT': os.path.realpath(options.colobot_lint_executable),
                        'DEBUG': '1' if options.debug_flag else '0' })