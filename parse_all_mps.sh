#!/bin/bash

# Directory containing MPS files (replace with actual path or use command-line argument)
# MPS_DIR="mps_files" # Placeholder directory
MPS_DIR="$HOME/.miplib_benchmark/mps_files"
# Check if the directory exists
if [ ! -d "$MPS_DIR" ]; then
  echo "Error: Directory not found: $MPS_DIR" >&2
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

# Find all .mps files in the specified directory and store them in an array
# Use find with -print0 and a while read loop for safety with special characters
MPS_FILES=()
while IFS= read -r -d $'\0' file; do
    MPS_FILES+=("$file")
done < <(find "$MPS_DIR" -maxdepth 1 -name '*.mps' -print0)

# Check if any MPS files were found
NUM_FILES=${#MPS_FILES[@]}
if [ $NUM_FILES -eq 0 ]; then
  echo "No .mps files found in $MPS_DIR."
  exit 0
fi

# Ask for user confirmation
echo "Found $NUM_FILES .mps file(s) in $MPS_DIR."
read -p "Proceed with parsing? (y/n): " CONFIRMATION
if [[ ! "$CONFIRMATION" =~ ^[Yy]$ ]]; then
  echo "Operation cancelled by user."
  exit 0
fi

# Process each MPS file found
echo "Starting processing..."
for MPS_FILE in "${MPS_FILES[@]}"; do
  # No need to trim null characters here as the loop handles it
  echo "Processing MPS file: $MPS_FILE"
  "$EXECUTABLE" "$MPS_FILE"

  # Check the exit status of the executable for each file
  EXIT_STATUS=$?
  if [ $EXIT_STATUS -ne 0 ]; then
    echo "Error: parse_and_save exited with status $EXIT_STATUS for file $MPS_FILE" >&2
    # Optionally, decide whether to stop on error or continue with the next file
    # exit $EXIT_STATUS # Uncomment to stop on first error
  fi
done

echo "Script finished processing all MPS files in $MPS_DIR."
exit 0 