import subprocess
import tempfile
import shutil
import sys
import unittest
import xml.etree.ElementTree as ET

colobot_lint_exectuable = './colobot-lint' # default

def run_colobot_lint(source_file_lines):
    temp_dir = tempfile.mkdtemp()

    try:
        source_file_name = temp_dir + '/src.cpp'
        with open(source_file_name, 'w') as f:
            f.write('\n'.join(source_file_lines))

        compilation_database_file_name = temp_dir + '/compile_commands.json'
        with open(compilation_database_file_name, 'w') as f:
            f.write(
                r"""[
                    {{
                        "directory": "{directory}",
                        "command": "/usr/bin/c++ -c -std=c++11 {source_file_name} -o {source_file_name}.o",
                        "file": "{source_file_name}"
                    }}
                ]""".format(
                    directory = temp_dir,
                    source_file_name = source_file_name)
                )

        return subprocess.check_output([colobot_lint_exectuable, '-p', temp_dir, source_file_name])
    finally:
        shutil.rmtree(temp_dir)

class TestBase(unittest.TestCase):
    def assert_colobot_lint_result(self, source_file_lines, expected_errors):
        xml_output = run_colobot_lint(source_file_lines)

        results = ET.fromstring(xml_output)
        errors = results.find('errors').findall('error')

        self.assertEqual(len(errors), len(expected_errors))

        for error, expected_error in zip(errors, expected_errors):
            self.assertEqual(error.get('id'), expected_error['id'])
            self.assertEqual(error.get('severity'), expected_error['severity'])
            self.assertEqual(error.get('msg'), expected_error['msg'])

            location = error.find('location')
            self.assertEqual(location.get('line'), expected_error['line'])