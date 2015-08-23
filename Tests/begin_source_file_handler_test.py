import test_support
import os

class BeginSourceFileHandlerTest(test_support.TestBase):
    def test_skip_multiple_files_in_compilation_database(self):
        with test_support.TempBuildDir() as temp_dir:
            cpp_file_name = temp_dir + '/src.cpp'
            test_support.write_file_lines(cpp_file_name, [
                    'void deleteMe(int* x)',
                    '{',
                    '   delete x;',
                    '}'
                ]
            )

            test_support.write_compilation_database(
                build_directory = temp_dir,
                source_file_names = [cpp_file_name, cpp_file_name],
                additional_compile_flags = '-I' + temp_dir)

            xml_output = test_support.run_colobot_lint(build_directory = temp_dir,
                                                       source_dir = temp_dir,
                                                       source_paths = [cpp_file_name],
                                                       rules_selection = ['NakedDeleteRule'])
            self.assert_xml_output_match(
                xml_output = xml_output,
                expected_errors = [
                    {
                        'id': 'naked delete',
                        'severity': 'warning',
                        'msg': "Naked delete called on type 'int'",
                        'line': '3'
                    }
                ]
            )
