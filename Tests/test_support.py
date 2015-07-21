import subprocess
import tempfile
import shutil
import sys
import unittest
import xml.etree.ElementTree as ET

colobot_lint_exectuable = './colobot-lint' # default

class TempBuildDir:
    def __init__(self):
        self.path = None

    def __enter__(self):
        self.path = tempfile.mkdtemp()
        return self.path

    def __exit__(self, type, value, traceback):
        shutil.rmtree(self.path)

def write_file_lines(file_name, lines):
    with open(file_name, 'w') as f:
        f.write('\n'.join(lines))

def write_compilation_database(build_directory, source_file_names, additional_compile_flags = ''):
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

def run_colobot_lint_with_single_file(source_file_lines):
    with TempBuildDir() as temp_dir:
        source_file_name = temp_dir + '/src.cpp'
        write_file_lines(source_file_name, source_file_lines)
        write_compilation_database(temp_dir, [source_file_name])
        return run_colobot_lint(temp_dir, [source_file_name])

def run_colobot_lint(build_directory, source_paths):
    return subprocess.check_output([colobot_lint_exectuable, '-p', build_directory] + source_paths)

class TestBase(unittest.TestCase):
    def assert_colobot_lint_result(self, source_file_lines, expected_errors):
        xml_output = run_colobot_lint_with_single_file(source_file_lines)
        self.assert_xml_output_match(xml_output, expected_errors)

    def assert_xml_output_match(self, xml_output, expected_errors):
        results = ET.fromstring(xml_output)
        errors = results.find('errors').findall('error')

        self.assertEqual(len(errors), len(expected_errors))

        for error, expected_error in zip(errors, expected_errors):
            self.assertEqual(error.get('id'), expected_error['id'])
            self.assertEqual(error.get('severity'), expected_error['severity'])
            self.assertEqual(error.get('msg'), expected_error['msg'])

            location = error.find('location')
            self.assertEqual(location.get('line'), expected_error['line'])