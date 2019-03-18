#!/usr/bin/env bash

# Cause the script to exit if a single command fails.
set -e
# Show explicitly which commands are currently running.
set -x

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE:-$0}")"; pwd)

# run this file before compile the targets
sh $ROOT_DIR/generate_deps.sh

echo "Compiling Java code."
pushd $ROOT_DIR/..
# Compile all the targets.
bazel build //java:all_modules --verbose_failures
# Lint Java code with checkstyle.
bazel test //java:all --test_tag_filters="checkstyle"

echo "Running tests under cluster mode."
ENABLE_MULTI_LANGUAGE_TESTS=1 bazel test //java:all_tests --test_output="errors" || cluster_exit_code=$?

# exit_code == 2 means there are some tests skiped.
if [ $cluster_exit_code -eq 2 ] && [ $cluster_exit_code -eq 0 ] ; then
    exit $exit_code
fi

echo "Running tests under single-process mode."
bazel test //java:all_tests --jvmopt="-Dray.run-mode=SINGLE_PROCESS" --test_output="errors" || single_exit_code=$?

# exit_code == 2 means there are some tests skiped.
if [ $single_exit_code -eq 2 ] && [ $single_exit_code -eq 0 ] ; then
    exit $exit_code
fi

popd
