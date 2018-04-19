#!/usr/bin/env groovy
pipeline {
    agent { label 'colobot-build' }
    options {
        buildDiscarder(logRotator(artifactNumToKeepStr: '5'))
    }
    stages {
        stage('Build') {
            steps {
                sh 'mkdir -p build'
                dir('build') {
                    sh '''
                        cmake \
                            -DCMAKE_BUILD_TYPE=Release -DTESTS=1 \
                            -DCMAKE_CXX_FLAGS="-I/usr/lib/llvm-3.6/include -L/usr/lib/llvm-3.6/lib" \
                            ..
                        make
                    '''
                }
            }
            post {
                success {
                    archiveArtifacts 'build/colobot-lint'
                }
            }
        }
        stage('Package HtmlReport') {
            steps {
                sh '''
                    rm -f build/html_report.tar.gz
                    tar -zcf build/html_report.tar.gz HtmlReport/
                '''
            }
            post {
                success {
                    archiveArtifacts 'build/html_report.tar.gz'
                }
            }
        }
        stage('Package count_errors.py') {
            steps {
                archiveArtifacts 'Tools/count_errors.py'
            }
        }
        stage('Run unit tests') {
            steps {
                dir('build') {
                    sh '''
                        echo "Cleaning previous tests"
                        if [ -e ./Testing ]; then
                            find ./Testing -name Test.xml -delete;
                        fi
                        echo "Running colobot-lint unit tests"
                        ctest --no-compress-output -T Test . || true
                    '''
                }
                step([$class: 'XUnitPublisher',
                    testTimeMargin: '3000',
                    thresholdMode: 1,
                    thresholds: [
                        [$class: 'FailedThreshold', failureNewThreshold: '', failureThreshold: '', unstableNewThreshold: '', unstableThreshold: '1'],
                        [$class: 'SkippedThreshold', failureNewThreshold: '', failureThreshold: '', unstableNewThreshold: '', unstableThreshold: '1']
                    ],
                    tools: [
                        [$class: 'CTestType', deleteOutputFiles: true, failIfNotNew: false, pattern: 'build/Testing/**/Test.xml', skipNoTestFiles: false, stopProcessingIfError: true]
                    ]
                ])
            }
        }
        stage('Test run on Colobot') {
            when { branch 'dev' }
            steps {
                dir('build') {
                    sh '''#!/bin/bash
set -e +x
# Update colobot workspace

COLOBOT_REPO_URL="https://github.com/colobot/colobot.git"
COLOBOT_DIR="./colobot"

if [ ! -e "$COLOBOT_DIR" ]; then
	echo "Cloning colobot"
    git clone --branch=dev --recursive "$COLOBOT_REPO_URL" "$COLOBOT_DIR"
else
	echo "Updating colobot"
	cd "$COLOBOT_DIR"
    git checkout dev
    git pull origin dev
    git submodule update
fi
                    '''
                    sh '''#!/bin/bash
set -e +x
# Run colobot-lint

COLOBOT_DIR="$WORKSPACE/build/colobot"
COLOBOT_BUILD_DIR="$WORKSPACE/build/colobot-build"

COLOBOT_LINT_BUILD_DIR="$WORKSPACE/build"

COLOBOT_LINT_REPORT_FILE="$WORKSPACE/build/colobot_lint_report.xml"

CLANG_PREFIX="/usr/lib/llvm-3.6"

echo "Running CMake for colobot"
rm -rf "$COLOBOT_BUILD_DIR"
mkdir -p "$COLOBOT_BUILD_DIR"
cd "$COLOBOT_BUILD_DIR"
cmake -DCOLOBOT_LINT_BUILD=1 -DTESTS=1 -DTOOLS=1 -DMUSIC=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 "$COLOBOT_DIR"

echo "Running colobot-lint for colobot"
cd "$COLOBOT_LINT_BUILD_DIR"

# Workaround for Clang not finding system headers
rm -rf bin/
mkdir -p bin
mv ./colobot-lint ./bin/
rm -f ./lib
ln -s ${CLANG_PREFIX}/lib ./lib

find "$WORKSPACE" \\( -wholename "$COLOBOT_DIR/src/*.cpp" \
                 -or -wholename "$COLOBOT_DIR/test/unit/*.cpp" \
                 -or -wholename "$COLOBOT_BUILD_DIR/fake_header_sources/src/*.cpp" \
                 -or -wholename "$COLOBOT_BUILD_DIR/fake_header_sources/test/unit/*.cpp" \\) \
                 -exec ./bin/colobot-lint \
                      -verbose \
                      -output-format xml \
                      -output-file "$COLOBOT_LINT_REPORT_FILE" \
                      -p "$COLOBOT_BUILD_DIR" \
                      -project-local-include-path "$COLOBOT_DIR/src" -project-local-include-path "$COLOBOT_BUILD_DIR/src" \
                      -license-template-file "$COLOBOT_DIR/LICENSE-HEADER.txt" \
                      {} +
                    '''
                    sh '''#!/bin/bash
set -e +x
# Update stable/unstable build status

COLOBOT_LINT_REPORT_FILE="$WORKSPACE/build/colobot_lint_report.xml"
COLOBOT_LINT_DIR="$WORKSPACE"

OVERALL_STABLE_RULES=(
    "class naming"
    "code block placement"
    "compile error"
#    "compile warning"
#    "enum naming"
#    "function naming"
    "header file not self-contained"
#    "implicit bool cast"
#    "include style"
#    "inconsistent declaration parameter name"
    "license header"
#    "naked delete"
#    "naked new"
#    "old style function"
    "old-style null pointer"
#    "possible forward declaration"
    "undefined function"
#    "uninitialized field"
#    "uninitialized local variable"
#    "unused forward declaration"
#    "variable naming"
    "whitespace"
)

echo "Checking rule stability (overall)"
for ((i = 0; i < ${#OVERALL_STABLE_RULES[@]}; i++)); do
    rule="${OVERALL_STABLE_RULES[$i]}"
    count="$("$COLOBOT_LINT_DIR/Tools/count_errors.py" --rule-filter="$rule" --xml-report-file "$COLOBOT_LINT_REPORT_FILE")"
    if [ "$count" != "0" ]; then
       echo "UNSTABLE RULE: $rule ($count occurences)"
    fi
done

STABLE_RULES_WITHOUT_CBOT=(
    "class naming"
    "code block placement"
    "compile error"
    "compile warning"
#    "enum naming"
#    "function naming"
    "header file not self-contained"
#    "implicit bool cast"
    "include style"
    "inconsistent declaration parameter name"
    "license header"
    "naked delete"
    "naked new"
#    "old style function"
    "old-style null pointer"
#    "possible forward declaration"
    "undefined function"
    "uninitialized field"
#    "uninitialized local variable"
    "unused forward declaration"
#    "variable naming"
    "whitespace"
)

echo "Checking rule stability (without CBOT)"
for ((i = 0; i < ${#STABLE_RULES_WITHOUT_CBOT[@]}; i++)); do
    rule="${STABLE_RULES_WITHOUT_CBOT[$i]}"
    count="$("$COLOBOT_LINT_DIR/Tools/count_errors.py" --rule-filter="$rule" --file-filter="-.*CBot.*" --xml-report-file "$COLOBOT_LINT_REPORT_FILE")"
    if [ "$count" != "0" ]; then
       echo "UNSTABLE RULE: $rule (without CBOT, $count occurences)"
    fi
done
		    '''
                }
            }
        }
    }
}
