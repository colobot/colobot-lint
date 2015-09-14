import test_support

class PossibleForwardDeclarationRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['PossibleForwardDeclarationRule'])
        self.set_default_error_id('possible forward declaration')
        self.set_default_error_severity('information')

    def assert_result_with_header_files(self,
                                        main_file_lines_without_includes,
                                        project_header_lines = [],
                                        indirect_project_header_lines = [],
                                        system_header_lines = [],
                                        expected_errors = []):
        main_file = 'src.cpp'
        project_header_file = 'project/header.h'
        indirect_project_header_file = 'project/indirect_header.h'
        system_header_file = 'system/header.h'

        project_header_lines_with_includes = [
                '#include <{0}>'.format(indirect_project_header_file)
            ] + project_header_lines

        main_file_lines_with_includes = [
                '#include "{0}"'.format(project_header_file),
                '#include <{0}>'.format(system_header_file)
            ] + main_file_lines_without_includes

        source_files_data = {
            main_file: main_file_lines_with_includes,
            project_header_file: project_header_lines_with_includes,
            indirect_project_header_file: indirect_project_header_lines,
            system_header_file: system_header_lines
        }

        self.assert_colobot_lint_result_with_custom_files(
            source_files_data = source_files_data,
            compilation_database_files = [main_file],
            target_files = [main_file],
            additional_compile_flags = ['-I$TEMP_DIR'],
            additional_options = ['-project-local-include-path', '$TEMP_DIR/project'],
            expected_errors = expected_errors)

    def test_forward_declaration_possible_with_reference_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo&);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [
                {
                    'msg': "Class 'Foo' can be forward declared instead of #included",
                    'line': '3'
                }
            ])

    def test_forward_declaration_possible_with_pointer_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [
                {
                    'msg': "Class 'Foo' can be forward declared instead of #included",
                    'line': '3'
                }
            ])

    def test_forward_declaration_impossible_with_whole_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_return_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'Foo FooFunc();'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_template_class(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(const FooClassTemplate<int>&);'
            ],
            project_header_lines = [
                'template<typename T> class FooClassTemplate {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_template_argument_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'template<typename T> class FooClassTemplate { void Foo(T); };',
                'void FooFunc(FooClassTemplate<Foo>&);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_old_style_enum(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(const Foo&);'
            ],
            project_header_lines = [
                'enum Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_possible_with_enum_class(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(const Foo&);'
            ],
            project_header_lines = [
                'enum class Foo : int {};'
            ],
            expected_errors = [
                {
                    'msg': "Enum class 'Foo' can be forward declared instead of #included",
                    'line': '3'
                }
            ])

    def test_ignore_already_forward_declared_type_in_project_header(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
            ],
            project_header_lines = [
                'class Foo;'
            ],
            expected_errors = [])

    def test_ignore_types_from_system_header_reference_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo&);'
            ],
            system_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_ignore_types_from_system_header_pointer_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
            ],
            system_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_ignore_types_from_indirect_project_header_reference_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo&);'
            ],
            indirect_project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_ignore_types_from_indirect_project_header_pointer_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
            ],
            indirect_project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])
