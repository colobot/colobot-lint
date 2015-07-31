# colobot-lint rules

This is a complete list of rules that are checked by colobot-lint.

Each rule is (usually) contained in its own class and can be enabled/disabled individually. To run only selected rules, use commandline argument(s) `-only-rule {rule class}`.

Basic working mechanism of every rule is to match selected AST tree elements and perform some checks, and if any violations are found, report them with appropriate error messages.

## General checks
This is a class of checks that are always turned on and they are handled in other ways than through regular rules.

## Compile error/warning

Errors:
 * [error] {compile error}
 * [warning] {compile warning}
 * [error] Including single header file should not result in compile error: {compile error}

On report of a compile diagnostic (error/warning), one of the above errors will be reported.

In regular files, these are simply verbatim reports of compile error/warning.

In case of single header files (checked through fake header source mechanism), any compile error due to (most likely) missing #include directives or forward declarations is also treated as an error.

## Rule exclusions

This checks for comments in code in form:
```cpp
   // @colobot-lint-exclude {rule class1} {rule class2} {...}
   // @end-colobot-lint-exclude
```

Between pair of such comments, any violations of the given rule classes are ignored.

You can also specify "*" to exclude all rules.

The ending comment must always be present and if it is not encountered before end of file is reached, the exclude directive is ignored.


# Initialization rules

## Uninitialized local variable
Class: UninitializedLocalVariableRule
Errors:
 - [error] Local variable '{name}' is uninitialized

This rule checks all function definitions for declarations local variables of POD types without initialization.

Note: this rule is intentionally inhibited in old-style functions (matched by OldStyleFunctionRule) to avoid flood of errors.

## Uninitialized field
Class: UninitializedFieldRule
Errors:
 - [error] Class/Struct '{name}' field '{name}' remains uninitialized
 - [error] Class/Struct '{name}' field '{name}' remains uninitialized in constructor

This rule matches all class/struct declarations and constructor definitions, checking that all POD type fields are initialized.

Allowed methods of initialization include:
 - direct initialization in class declaration
 - constructor initialization list
 - simple assignment "{field} = {expr}" in constructor body

Note: for simplicity, assignment statements are not checked recursively in constructor body.

# Memory management rules

## Naked new

Class: NakedNewRule
Errors:
 - [warning] Naked new called with type '{name}'

This rule matches any call of new expression.

## Naked delete

Class: NakedDeleteRule
Errors:
 - [warning] Naked delete called on type '{name}'

This rule matches any call of delete expression.

# Information rules

## Old-style function

Class: OldStyleFunctionRule
Errors:
 - [information] Function '%s' has variables declared far from point of use ('var1', 'var2', ...)

This rule matches all function and method definitions in code, checking for functions written in legacy C style:
```
void Foo()
{
    int localVar1, localVar2;
    float localVar3;

    // ...

    localVar1 = IsUsedManyManyLinesAfterItsDeclaration();
}
```

## TODO

Class: TodoRule
Errors:
 - [information] TODO comment: {comment}

This rule checks for all TODO comments in code.

# Style rules

## Block placement

Class: BlockPlacementRule
Errors:
 - [style] Body of declaration or statement begins/ends in a style that is not allowed

This rule finds incorrect placement of "{" abd "}" braces surrounding blocks of code. The allowed style is to place both opening brace and closing brace in new lines, or both of them in one line, forming one-liners:
 - case #1 - separate lines:

```cpp
    { // <- opening brace - only whitespace allowed before it
        // zero or more lines in code block
    } // <- closing brace - only whitespace allowed before it
```
 - case #2 - one-liners:
```cpp
    { /* one-liner code block */ }
```

This rule is appplied to code blocks with statements and declarations of namespaces, classes/structs/unions.

## Class naming

Class: ClassNamingRule
Errors:
 - [information] Anonymous class/struct/union
 - [style] Class '{name}' should be named in a style like CUpperCamelCase
 - [style] Struct/Union '{name}' should be named in a style like UpperCamelCase
 - [style] Struct/Union '{name}' follows class naming style CUpperCamelCase but is not a class

This rule checks names of all declared classes/structs/unions. Allowed names are as evident in error messages above.

Additionally, anonymous structs/unions are reported.

## Enum naming

Class: EnumNamingRule
Errors:
 - [information] Old-style enum '{name}'
 - [style] Enum class '{name}' should be named in a style like UpperCamelCase
 - Enum class constant '{name}' should be named in a style like UpperCamelCase

This rule checks names of all declared enum classes. Allowed names are as evident in error messages above.

Additionally, old-style (not class) enums are reported.

## Function naming

Class: FunctionNamingRule
Errors:
 - [style] Function/Method '{name}' should be named in UpperCamelCase style

This rule checks names of all declared free functions and class methods. Allowed names are as evident in error message.

Notes:
 - exception is granted to methods named "begin" and "end" to enable C++11 ranged iteration
 - virtual override methods in derived classes are not reported, only the overridden base class method
 - each function name is reported only once, even if declared multiple times

# Variable naming

Class: VariableNamingRule
Errors:
 - [style] Local variable '{name}' should be named in camelCase style
 - [style] Local variable '{name}' is named in a style that is deprecated
 - [style] Const global variable '{name}' should be named in ALL_CAPS style
 - [style] Non-const global variable '{name}' should be named in g_camelCase style
 - [style] Public field '{name}' should be named in camelCase style
 - [style] Private/Protected field '{name}' should be named in m_camelCase style
 - [style] Private/Protected field '{name}' is named in a style that is deprecated

This rule checks names of all declared variables, function parameter names and class/struct/union fields. Allowed names are as evident in error messages.

Additionally, deprecated style names like "bBooleanType", "pPointerType", "m_bBooleanType", "m_pPointerType" are also reported.

# Inconsistent declaration parameter name

Class: InconsistentDeclarationParameterNameRule
Errors:
 - [style] Function '{name}' has other declaration(s) with inconsistently named parameter(s)

This rule checks all function and method declarations, including all their re-declarations appearing in translation unit. If one of these declarations has a parameter that is named differently than in any other declaration, it is reported.

Notes:
 - empty parameter names are skipped
 - each function is only reported once

## Include style

Class: IncludeStyleRule
Errors:
 - [style] Expected first include directive to be matching header file: '{expected name}', not '{actual name}'
 - [style] Expected config include directive: '{expected name}', not '{actual name}'
 - [style] Include '{name}' breaks alphabetical ordering
 - [style] Local include '{name}' should not be placed after global includes
 - [style] Expected empty line between include directives
 - [style] Local include '{name}' should be included with quotes, not angled brackets
 - [style] Global include '{name}' should be included with angled brackets, not quotes
 - [style] Expected local include to be full relative path from project local include search path: '{expected name}', not '{actual name}'

This rule checks correct style of #include directives. The allowed ordering and style of directives is as in following example:
```cpp
#include "matching_header_file.h" // if applicable (cpp files)
#include "base_class_of_derived_class_declared_here.h" // if applicable (header files)

#include "common/config.h" // if applicable

#include "local_include1_alphabetical_ordering.h"
#include "local_include2_alphabetical_ordering.h"
// ...
#include "local_includeN_alphabetical_ordering.h"

#include <global_include.h>
#include <global_include_order_doesnt_matter.h>
```

Besides ordering, it is also checked that:
 - quotes are used for local includes, angled brackets for global includes
 - empty lines divide certain include blocks, as in the example
 - local include paths use full relative paths, for example `common/resources/resource_manager.h`, not `resource_manager.h`, even if the included file is in same directory

Note: local vs global includes are distinguished based on compiler include paths given with commandline argument(s) `-project-local-include-path {include path}`. These paths are also used to generate expected local include relative paths.

## Whitespace rule

Class: WhitespaceRule
Errors:
 - [style] File seems to have DOS style line endings
 - [style] Whitespace at end of line
 - [style] Tab character is not allowed as whitespace
 - [style] File should end with newline

These should be evident from messages of reported errors.