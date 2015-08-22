import subprocess
import tempfile
import shutil
import sys
import os
import unittest
import xml.etree.ElementTree as ET

colobot_lint_exectuable = 'colobot-lint' # default
if 'COLOBOT_LINT' in os.environ:
    colobot_lint_exectuable = os.environ['COLOBOT_LINT']

debug_flag = False # default
if 'DEBUG' in os.environ:
    debug = os.environ['DEBUG'] == '1'

class TempBuildDir:
    def __init__(self):
        self.path = None

    def __enter__(self):
        self.path = tempfile.mkdtemp()
        return self.path

    def __exit__(self, type, value, traceback):
        if debug_flag:
            print("Temporary files left in: " + self.path)
        else:
            shutil.rmtree(self.path)

def write_file_lines(file_name, lines):
    with open(file_name, 'w') as f:
        f.write('\n'.join(lines))

def write_compilation_database(build_directory,
                               source_file_names,
                               additional_compile_flags = ''):
    compilation_database_file_name = build_directory + '/compile_commands.json'
    with open(compilation_database_file_name, 'w') as f:
        f.write("[\n")

        comma = False

        for source_file_name in source_file_names:
            if comma:
                f.write(",")

            f.write(r"""
                {{
                    "directory": "{build_directory}",
                    "command": "/usr/bin/c++ -c -std=c++11 {additional_compile_flags} {source_file_name} -o {source_file_name}.o",
                    "file": "{source_file_name}"
                }}
                """.format(
                    build_directory = build_directory,
                    additional_compile_flags = additional_compile_flags,
                    source_file_name = source_file_name)
            )

            comma = True

        f.write("]\n")

def run_colobot_lint_with_single_file(source_file_lines,
                                      rules_selection,
                                      additional_options = [],
                                      additional_compile_flags = ''):
    with TempBuildDir() as temp_dir:
        source_file_name = temp_dir + '/src.cpp'
        write_file_lines(source_file_name, source_file_lines)
        write_compilation_database(temp_dir, [source_file_name], additional_compile_flags)
        return run_colobot_lint(build_directory = temp_dir,
                                source_dir = temp_dir,
                                source_paths = [source_file_name],
                                rules_selection = rules_selection,
                                additional_options = additional_options)

def run_colobot_lint(build_directory,
                     source_dir,
                     source_paths,
                     rules_selection = [],
                     additional_options = []):
    rules_selection_options = []
    for rule in rules_selection:
        rules_selection_options += ['-only-rule', rule]

    whole_command = ([colobot_lint_exectuable] +
                     ['-output-format', 'xml'] +
                     rules_selection_options +
                     additional_options +
                     ['-p', build_directory] +
                     ['-project-local-include-path', source_dir] +
                     source_paths)

    if debug_flag:
        print("Running colobot-lint command:")
        print(whole_command)

    command_output = subprocess.check_output(whole_command)

    if debug_flag:
        print("Command output is:")
        print(command_output.decode('utf-8'))

    return command_output


class TestBase(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.default_rules_selection = []
        self.default_error_id = ''
        self.default_error_severity = ''

    def set_default_rules_selection(self, rules_selection):
        self.default_rules_selection = rules_selection

    def set_default_error_id(self, error_id):
        self.default_error_id = error_id

    def set_default_error_severity(self, error_severity):
        self.default_error_severity = error_severity

    def assert_colobot_lint_result(self,
                                   source_file_lines,
                                   expected_errors,
                                   rules_selection = None,
                                   additional_options = [],
                                   additional_compile_flags = ''):
        rules_sel = []
        if rules_selection is not None:
            rules_sel = rules_selection
        else:
            rules_sel = self.default_rules_selection
        xml_output = run_colobot_lint_with_single_file(
            source_file_lines = source_file_lines,
            rules_selection = rules_sel,
            additional_options = additional_options,
            additional_compile_flags = additional_compile_flags)

        self.assert_xml_output_match(xml_output, expected_errors)

    def assert_xml_output_match(self, xml_output, expected_errors):
        results = ET.fromstring(xml_output)
        errors = results.find('errors').findall('error')

        self.assertEqual(len(errors), len(expected_errors))

        for error, expected_error in zip(errors, expected_errors):
            self.assertEqual(error.get('id'),
                             expected_error.get('id', self.default_error_id))
            self.assertEqual(error.get('severity'),
                             expected_error.get('severity', self.default_error_severity))
            self.assertEqual(error.get('msg'), expected_error['msg'])

            location = error.find('location')
            self.assertEqual(location.get('line'), expected_error['line'])
