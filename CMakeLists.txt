set(LLVM_LINK_COMPONENTS support)
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

add_clang_executable(colobot-lint
  ColobotLint.cpp
  ActionFactories.cpp
  DiagnosticHandler/DiagnosticHandler.cpp
  Common/OutputPrinter.cpp
  Rules/BraceInNewLineRule.cpp
  Rules/NakedDeleteRule.cpp
  Rules/NakedNewRule.cpp
  Rules/TodoRule.cpp
  Rules/RulesFactory.cpp
)
target_link_libraries(colobot-lint
  clangTooling
  clangBasic
  clangASTMatchers
  tinyxml
)

set(GIT_COMMIT_HASH "<none>" CACHE STRING "Latest git commit hash used as version")

configure_file(Common/ColobotLintConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/ColobotLintConfig.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "Git commit hash set to ${GIT_COMMIT_HASH}")
