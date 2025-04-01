#!/bin/bash

# Find the first MPS file in the mps_files directory
MPS_FILE=$(find mps_files -maxdepth 1 -name '*.mps' -print -quit)

# Check if an MPS file was found
if [ -z "$MPS_FILE" ]; then
  echo "Error: No .mps file found in mps_files/ directory." >&2
  exit 1
fi

# Define the path to the executable
EXECUTABLE="./build/src/parse_and_save"

# Check if the executable exists
if [ ! -x "$EXECUTABLE" ]; then
  echo "Error: Executable not found or not executable: $EXECUTABLE" >&2
  echo "Please build the project first using ./build.sh" >&2
  exit 1
fi

# Run the executable with the found MPS file
echo "Using MPS file: $MPS_FILE"
"$EXECUTABLE" "$MPS_FILE"

# Check the exit status of the executable
EXIT_STATUS=$?
if [ $EXIT_STATUS -ne 0 ]; then
  echo "Error: parse_and_save exited with status $EXIT_STATUS" >&2
  exit $EXIT_STATUS
fi

echo "Script finished successfully."
exit 0 