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
                '#include <{0}>'.format(system_header_file),
                '#include "{0}"'.format(project_header_file)
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
                'void FooFunc1(const Foo&);',
                'void FooFunc2(Foo);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_return_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc1(const Foo&);',
                'Foo FooFunc2();'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_type_used_as_field(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'class Bar',
                '{',
                '    Foo foo;',
                '};',
                'void FooFunc(Foo&);'
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
                'void FooFunc(FooClassTemplate<Foo>&);',
                'void FooFunc(Foo&);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_return_template_argument_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'template<typename T> class FooClassTemplate { void Foo(T); };',
                'FooClassTemplate<Foo> FooFunc();',
                'void FooFunc(Foo&);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_elaborated_template_argument_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'namespace NS',
                '{',
                '  template<typename T> class FooClassTemplate { void Foo(T); };',
                '}',
                'void FooFunc(NS::FooClassTemplate<Foo>&);',
                'void FooFunc(Foo&);'
            ],
            project_header_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_forward_declaration_impossible_with_array_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'class Bar',
                '{',
                '   Foo foo[4];',
                '};',
                'void FooFunc(Foo&);'
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

    def test_forward_declaration_impossible_with_typedef(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(FooConstRefAlias);'
            ],
            project_header_lines = [
                'class Foo {};',
                'typedef const Foo& FooConstRefAlias;'
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

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_whole_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);',
                'void BarFunc(Bar);',
            ],
            project_header_lines = [
                'class Foo {};',
                'class Bar {};'
            ],
            expected_errors = [])

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_base_class(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);',
                'class Derived : public Base {};'
            ],
            project_header_lines = [
                'class Foo {};',
                'class Base {};'
            ],
            expected_errors = [])

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_typedef(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);',
                'void BarFunc(BarAlias);',
            ],
            project_header_lines = [
                'class Foo {};',
                'class Bar {};',
                'typedef Bar BarAlias;'
            ],
            expected_errors = [])

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_using_template_alias(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);',
                'void BarFunc(BarAlias&);'
            ],
            project_header_lines = [
                'class Foo {};',
                'class Bar {};',
                'using BarAlias = ClassTemplate<Bar>;'
            ],
            system_header_lines = [
                'template<typename T>',
                'class ClassTemplate {};'
            ],
            expected_errors = [])

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_use_of_const(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);',
                'const int ANOTHER_GLOBAL = GLOBAL + 1;',
            ],
            project_header_lines = [
                'class Foo {};',
                'const int GLOBAL = 1;'
            ],
            expected_errors = [])

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_old_style_enum(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
                'void BarFunc(Bar&);'
            ],
            project_header_lines = [
                'class Foo {};',
                'enum Bar {};'
            ],
            expected_errors = [])

    def test_blacklist_project_header_with_types_which_cannot_be_forward_declared_template_class(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
                'void BarFunc(Bar<int>&);'
            ],
            project_header_lines = [
                'class Foo {};',
                'template<typename T>',
                'class Bar {};'
            ],
            expected_errors = [])

    def test_dont_blacklist_project_header_if_it_defines_not_used_non_forward_declarable_type(self):
        self.assert_result_with_header_files(
            main_file_lines_without_includes = [
                'void FooFunc(Foo*);'
            ],
            project_header_lines = [
                'class Foo {};',
                'class Bar {};',
                'typedef Bar BarAlias;'
            ],
            expected_errors = [
                {
                    'msg': "Class 'Foo' can be forward declared instead of #included",
                    'line': '3'
                }
            ])
