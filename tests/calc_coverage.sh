#!/bin/bash
# By default no flags are passed to genn-buildmodel.sh
BUILD_FLAGS=""
LCOV=0

# Parse command line arguments
OPTIND=1
while getopts "cl" opt; do
    case "$opt" in
    c)  BUILD_FLAGS="-c"
        ;;
    l) LCOV=1
        ;;
    esac
done

# Clean GeNN library
pushd $GENN_PATH/lib
make clean
popd

# Loop through feature tests
for f in features/*;
    do
        echo "Calculating code generation coverage of $f..."

        # Push feature directory
        pushd $f

        # Clean
        make clean &>msg

        # Build and generate model (measuring coverage)
        genn-buildmodel.sh $BUILD_FLAGS -v model.cc &>msg || exit $?

        # Pop feature directory
        popd
    done;

if [ $LCOV -eq 1 ]; then
  echo "Combining coverage data..."

  # Combine all GCOV ouratput in child directories
  lcov --directory . --capture --output-file coverage.txt -rc lcov_branch_coverage=1 &>msg

  echo "Generating HTML coverage report..."

  # Generate browseable HTML
  genhtml coverage.txt --branch-coverage --output-directory ./code_coverage_report/ &>msg
fi