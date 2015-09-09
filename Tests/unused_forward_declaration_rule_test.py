import test_support
import unittest

class UnusedForwardDeclarationRuleTest(test_support.TestBase):
    def setUp(self):
        self.set_default_rules_selection(['UnusedForwardDeclarationRule'])
        self.set_default_error_id('unused forward declaration')
        self.set_default_error_severity('information')

    def test_class_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo {};'
            ],
            expected_errors = [])

    def test_class_declared_and_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo;',
                'class Foo {};'
            ],
            expected_errors = [])

    def test_class_declared_and_referenced(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo;',
                'void FooFunc(Foo&);'
            ],
            expected_errors = [])

    def test_class_declared_but_not_referenced(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo;',
                'class Bar;',
                'void BarFunc(Bar&);'
            ],
            expected_errors = [
                {
                    'msg': "Unused forward declaration of class 'Foo'",
                    'line': '1'
                }
            ])

    def test_class_declaration_repeated(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo;',
                'class Foo;',
                'void FooFunc(Foo&);'
            ],
            expected_errors = [
                {
                    'msg': "Repeated forward declaration of class 'Foo'",
                    'line': '2'
                }
            ])

    def test_class_referenced_as_pointer(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo;',
                'void FooFunc(Foo*);'
            ],
            expected_errors = [])

    def test_class_referenced_as_return_type(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo;',
                'Foo FooFunc();'
            ],
            expected_errors = [])

    def test_class_referenced_as_template_argument(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'template<typename T> class ClassTemplate { void Foo(T); };',
                'class Foo;',
                'void FooFunc(ClassTemplate<Foo>);'
            ],
            expected_errors = [])

    def test_class_referenced_as_subtemplate_argument(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'template<typename T> class ClassTemplate { void Foo(T); };',
                'class Foo;',
                'void FooFunc(ClassTemplate<ClassTemplate<Foo>>);'
            ],
            expected_errors = [])

    def test_enum_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Foo : int {};'
            ],
            expected_errors = [])

    def test_enum_declared_and_defined(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Foo;',
                'enum class Foo : int {};'
            ],
            expected_errors = [])

    def test_enum_declared_but_not_referenced(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Foo : int;'
            ],
            expected_errors = [
                {
                    'msg': "Unused forward declaration of enum class 'Foo'",
                    'line': '1'
                }
            ])

    def test_enum_declared_and_referenced(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'enum class Foo : int;',
                'void FooFunc(Foo);',
                'enum class Bar : int {};',
                'void BarFunc(Bar);'
            ],
            expected_errors = [])

    def test_class_declaration_repeated_after_definition(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo { };',
                'class Foo;'
            ],
            expected_errors = [
                {
                    'msg': "Redundant forward declaration after definition of class 'Foo'",
                    'line': '2'
                }
            ])

    def test_ignore_friend_class_declaration(self):
        self.assert_colobot_lint_result(
            source_file_lines = [
                'class Foo { friend class Bar; };'
            ],
            expected_errors = [])
