# colobot-lint

This is a static analysis tool designed for use with source code of [open-source Colobot project](http://github.com/colobot/colobot).

It currently implements a few checks for consistent coding style and potentially problematic areas of code. Although it is designed specifically to help in development of Colobot, you may find it generic enough to use in your own projects. If so, you are welcome to do it.

It uses Clang's extensive API to implement a tool that "sees" the code exactly as the compiler does, which lets it directly analyze AST generated by compiler.

If you're not familiar with the idea of tools built on Clang's API (LibTooling), you can get started by reading [a few tutorials](http://clang.llvm.org/docs/LibASTMatchersTutorial.html).

## Supported checks

List of rules that colobot-lint checks is documented in [RULES.md file](RULES.md).

## Compiling

There are two possible ways of compiling colobot-lint:
 - along with Clang and LLVM sources,
 - as standalone project using installed Clang headers and libraries.

The recommmended method is the first one, as it is failsafe and will work on any system.
The second method is so far very experimental and probably only works on the only system it was tested on (Arch Linux).

## Requirements

You obviously need Clang, either its source files (see below), or have it installed normally on your system.
Please note that the only supported versions of Clang are 3.6 and 3.7, and it will probably stay that way to avoid hassle with changing APIs.

The other requirements necessary for compilation are basically same as for Clang (see for example [this](http://llvm.org/docs/GettingStarted.html)).
However, you will also need:
 - CMake >= 2.8
 - tinyxml >= 2.6
 - Boost regex >= 1.49

### Compiling along with Clang and LLVM sources

Download the following files from [LLVM project download page](http://llvm.org/releases/download.html):
 - `llvm-3.6*.tar.xz` (LLVM sources)
 - `cfe-3.6*.tar.xz` (Clang sources)
 - `clang-tools-extra-3.6*.tar.xz` (Clang Tools Extra)

Now unpack them as follows:
```
 $ tar -Jxf llvm-3.6*.tar.xz
 $ tar -Jxf cfe-3.6*.tar.xz
 $ tar -Jxf clang-tools-extra-3.6*.tar.xz
 $ mv llvm-3.6*/ llvm
 $ mv cfe-3.6*/ llvm/tools/clang
 $ mv clang-tools-extra-3.6*/ llvm/tools/clang/tools/extra
```

Clone this repository:
```
 $ git clone https://github.com/colobot/colobot-lint.git
```

Create symlinks in clang sources and add it to appropriate CMakeLists.txt:
```
 $ ln -s $PWD/colobot-lint llvm/tools/clang/tools/extra/colobot-lint
 $ echo "add_subdirectory(colobot-lint)" >> llvm/tools/clang/tools/extra/CMakeLists.txt
```

Now you can build everything together:
```
 $ mkdir clang-build
 $ cd clang-build
 $ cmake -DCMAKE_BUILD_TYPE=Release ../llvm
 $ make
```

The binary should be created in `./bin/colobot-lint`.

### Compiling standalone

Try this and keep your fingers crossed:
```
 $ git clone https://github.com/colobot/colobot-lint.git
 $ mkdir colobot-lint-build
 $ cd colobot-lint-build
 $ cmake -DCMAKE_BUILD_TYPE=Release ../colobot-lint
 $ make
```

The binary should be saved in `./colobot-lint`.

### Installing

When you run colobot-lint by executing the compiled binary, you will probably see errors about Clang being unable to find some system header files like `stdarg.h`. This is caused by Clang libraries searching for certain system header files relative to binary file: `../lib/clang/<version>/include/`.

There are two solutions to this problem:
 * install colobot-lint binary in the same place where you have other clang binaries (usually `/usr/bin`)
 * create a fake directory structure to force Clang libraries to search the path we want:
  - `bin/`
    - `colobot-lint` (colobot-lint binary; it must be a copy of the file, symlink won't work here)
  - `lib/ -> /usr/lib` (symlink to lib directory expected by Clang libraries; may be other path than `/usr/lib` depending on system)

## Running

To run the tool, you first need to generate compilation database for Colobot:
```
 $ git clone --recursive https://github.com/colobot/colobot.git
 $ mkdir colobot-build
 $ cd colobot-build
 $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -COLOBOT_LINT_BUILD=1 ../colobot
 $ cd ..
```

This is enough, you don't need to run `make` itself. Everything that this tool needs is file `compile_commands.json` which should appear inside `colobot-build`.

Now we can run the tool itself:
```
 $ colobot-lint -verbose -p "$PWD/colobot-build" -project-local-include-path "$PWD/colobot/src" -project-local-include-path "$PWD/colobot-build/src" -license-template-file "$PWD/colobot/LICENSE-HEADER.txt" $(find "$PWD/colobot/src" -name '*.cpp') $(find "$PWD/colobot-build/fake_header_sources" -name '*.cpp')
```

Notes:
 - remember the use of `$PWD` - you need to supply full paths to files, exactly as they appear in `compile_commands.json`, otherwise colobot-lint will complain that it cannot find the given file in compilation database,
 - if you're wondering what these `fake_header_sources` are, please read [note on header handling in RULES.md](RULES.md#note-on-header-handling)

The ouput from this command will be a list of violations printed to standard output in simple plain text.

There is also option `-output-format xml` for generating XML report in format compatible with that of cppcheck. This is so that you can use it with cppcheck plugin for Jenkins.

To save the report to a file instead of standard output, you can use option `-output-file <file name>`.

## Generating HTML report

To work around shortcomings of cppcheck plugin, an additional script is provided to generate report as interactive HTML page. The script takes previously generated XML file and saves the resulting HTML to specified directory. This is how you might invoke the script:
```
 $ ./HtmlReport/generate.py --xml-report-file my_report.xml --output-dir my_html_report
```

## Generating graphs

colobot-lint allows also for generation of two types of graphs based on information from all processed files. These graphs are:
 - dependency graph: shows which classes depend on which other classes and also which classes inherit from which other classes (dependency `A -> B` means: method of `A` calls a method of `B`),
 - deployment graph: shows which classes own instances of which other classes and also which classes inherit from which other classes (ownership `A -> B` means: `A` has field of type `std::unique_ptr<B>`).

When generating a graph, normal rule processing is not done and options related to rules do not apply.

The option to select graph is `-generate-graph <graph type>` where `<graph type>` is one of `DependencyGraph` or `DeploymentGraph`. The output is generated in `dot` format (`dot` is a program from [graphviz toolkit](http://www.graphviz.org/)). The resulting file can be later processed to an image.

Example:
```
 $ colobot-lint -verbose -generate-graph DeploymentGraph -output-file graph.dot -p "$PWD/colobot-build" $(find "$PWD/colobot/src" -name '*.cpp')
 $ dot -Tpng graph.dot -o graph.png
```

## Running unit tests

To be able to run unit tests for colobot-lint, you need to (re-)run CMake with option `-DTESTS=1` in build directory like so:
```
 $ cmake -DTESTS=1 ../colobot-lint
```

Now to execute all testcases, you can run special make target:
```
 $ make check-colobot-lint
```

To run only selected testcases, you can pass additional FILTER option:
```
 $ make check-colobot-lint FILTER="*.testcase_name"
```

There is also a debug flag which allows to see more information about what happens in given testcase:
```
 $ make check-colobot-lint FILTER="*.testcase_name" DEBUG=1
```

## License
colobot-lint is licensed under BSD license (see [LICENSE.txt file](LICENSE.txt)).
