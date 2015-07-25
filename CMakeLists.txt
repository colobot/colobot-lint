cmake_minimum_required(VERSION 2.8.8)

if("${PROJECT_NAME}" STREQUAL "LLVM")
    message(STATUS "Building colobot-lint in source with LLVM")
else()
    message(STATUS "Building colobot-lint as standalone project")
    message(STATUS "NOTE: this is experimental and may not work on many systems")

    project(colobot-lint)
endif()


# Packages

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
find_package(TinyXML REQUIRED)
find_package(Git REQUIRED)

if(NOT "${PROJECT_NAME}" STREQUAL "LLVM")
    find_package(ZLIB REQUIRED)

    option(SKIP_CURSES "Whether to skip finding curses" OFF)
    if(NOT SKIP_CURSES)
       find_package(Curses REQUIRED)
    endif()

    set(Boost_ADDITIONALVERSION "1.49" "1.49.0")
    find_package(Boost COMPONENTS regex REQUIRED)

    option(TESTS "Enable tests" OFF)
    if(TESTS)
        enable_testing()
    endif()
endif()


# Version check using Git

set(COLOBOT_LINT_GIT_COMMIT_HASH "<none>" CACHE STRING "Latest git commit hash used as version")
execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE COLOBOT_LINT_GIT_COMMIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Version set from git commit hash: ${COLOBOT_LINT_GIT_COMMIT_HASH}")


# Sources

set(COLOBOT_LINT_SRCS
    ColobotLint.cpp
    ActionFactories.cpp
    Common/OutputPrinter.cpp
    Common/SourceLocationHelper.cpp
    Handlers/BeginSourceFileHandler.cpp
    Handlers/DiagnosticHandler.cpp
    Rules/BlockPlacementRule.cpp
    Rules/NakedDeleteRule.cpp
    Rules/NakedNewRule.cpp
    Rules/RulesFactory.cpp
    Rules/TodoRule.cpp
    Rules/VariableNamingRule.cpp
    Rules/FunctionNamingRule.cpp
)

# The configure file

configure_file(Common/ColobotLintConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/ColobotLintConfig.h)

# Include dirs

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TINYXML_INCLUDE_DIRS}
)


if("${PROJECT_NAME}" STREQUAL "LLVM")
    set(LLVM_LINK_COMPONENTS support)
    set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

    add_clang_executable(colobot-lint ${COLOBOT_LINT_SRCS})

    target_link_libraries(colobot-lint
        clangTooling
        clangBasic
        clangASTMatchers
        ${TINYXML_LIBRARIES}
        ${Boost_LIBRARIES})
else()
    add_executable(colobot-lint ${COLOBOT_LINT_SRCS})

    add_definitions(
        -DCLANG_ENABLE_ARCMT
        -DCLANG_ENABLE_OBJC_REWRITER
        -DCLANG_ENABLE_STATIC_ANALYZER
        -DGTEST_HAS_RTTI=0
        -D_GNU_SOURCE
        -D__STDC_CONSTANT_MACROS
        -D__STDC_FORMAT_MACROS
        -D__STDC_LIMIT_MACROS
    )

    include_directories(
        ${ZLIB_INCLUDE_DIRS}
        ${CURSES_INCLUDE_DIRS}
    )

    macro(add_cmake_cxx_flag flag)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endmacro()

    add_cmake_cxx_flag("-pthread")
    add_cmake_cxx_flag("-fPIC")
    add_cmake_cxx_flag("-fvisibility-inlines-hidden")
    add_cmake_cxx_flag("-Wall")
    add_cmake_cxx_flag("-W")
    add_cmake_cxx_flag("-Wno-unused-parameter")
    add_cmake_cxx_flag("-Wwrite-strings")
    add_cmake_cxx_flag("-Wcast-qual")
    add_cmake_cxx_flag("-Wno-missing-field-initializers")
    add_cmake_cxx_flag("-pedantic")
    add_cmake_cxx_flag("-Wno-long-long")
    add_cmake_cxx_flag("-Wno-maybe-uninitialized")
    add_cmake_cxx_flag("-Wnon-virtual-dtor")
    add_cmake_cxx_flag("-Wno-comment")
    add_cmake_cxx_flag("-std=c++11")
    add_cmake_cxx_flag("-ffunction-sections")
    add_cmake_cxx_flag("-fdata-sections")
    add_cmake_cxx_flag("-fno-common")
    add_cmake_cxx_flag("-Woverloaded-virtual")
    add_cmake_cxx_flag("-fno-strict-aliasing")
    add_cmake_cxx_flag("-fno-exceptions")
    add_cmake_cxx_flag("-fno-rtti")

    target_link_libraries(colobot-lint
        LLVMSupport
        clangTooling
        clangBasic
        clangASTMatchers
        clangFrontend
        clangDriver
        LLVMOption
        clangParse
        LLVMMCParser
        clangSerialization
        clangSema
        clangEdit
        clangAnalysis
        clangAST
        LLVMBitReader
        clangToolingCore
        clangRewrite
        clangLex
        clangBasic
        LLVMCore
        LLVMMC
        LLVMSupport
        ${TINYXML_LIBRARIES}
        ${Boost_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${CURSES_LIBRARIES}
        dl
    )
endif()

set(TESTS
    Test/todo_rule_test.py
)

if("${PROJECT_NAME}" STREQUAL "LLVM")
    set(COLOBOT_LINT_OUTPUT_BINARY ${LLVM_BINARY_DIR}/bin/colobot-lint)
else()
    set(COLOBOT_LINT_OUTPUT_BINARY ${CMAKE_CURRENT_BINARY_DIR}/colobot-lint)
endif()

macro(add_colobot_lint_test test_name test_target)
    add_test(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/${test_target} --colobot-lint-exec ${COLOBOT_LINT_OUTPUT_BINARY})
endmacro()

add_colobot_lint_test(TodoRuleTest Tests/todo_rule_test.py)
add_colobot_lint_test(BracePlacementRuleTest Tests/block_placement_rule_test.py)
add_colobot_lint_test(NakedNewRuleTest Tests/naked_new_rule_test.py)
add_colobot_lint_test(NakedDeleteRuleTest Tests/naked_delete_rule_test.py)
add_colobot_lint_test(FakeHeaderSourceTest Tests/fake_header_source_test.py)
add_colobot_lint_test(VariableNamingRule Tests/variable_naming_rule_test.py)
