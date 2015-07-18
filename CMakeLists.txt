set(LLVM_LINK_COMPONENTS support)
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

add_clang_executable(colobot-lint
  ColobotLint.cpp
  DiagnosticChecker/DiagnosticChecker.cpp
  AstMatcherRules/NakedDeleteRule.cpp
  Utils/OutputPrinter.cpp
)
target_link_libraries(colobot-lint
  clangTooling
  clangBasic
  clangASTMatchers
  tinyxml
)

set(GIT_COMMIT_HASH "<none>" CACHE STRING "Latest git commit hash used as version")

configure_file(ColobotLintConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/ColobotLintConfig.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
