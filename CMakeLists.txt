set(LLVM_LINK_COMPONENTS support)
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

add_clang_executable(colobot-lint
  ColobotLint.cpp
  DiagnosticChecker/DiagnosticChecker.cpp
  Rules/NakedDeleteRule.cpp
  Utils/OutputPrinter.cpp
)
target_link_libraries(colobot-lint
  clangTooling
  clangBasic
  clangASTMatchers
  tinyxml
)
