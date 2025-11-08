#!/bin/bash

echo "==> 🛠️ Compiling..."
make || { echo "Compilation failed"; exit 1; }

echo -e "\n==> ✅ Test: Load signatures and detect in 'new_suspectfile'..."

# Restore fresh test file
cp new_suspectfile suspectfile

# Run the antivirus tool and simulate input for: Load -> test-signatures.bin, then Detect
OUTPUT=$(./virusDetector suspectfile <<EOF
1
test-signatures.bin
3
5
EOF
)

echo "$OUTPUT"

# Check if detection was successful
echo "$OUTPUT" | grep -q "Starting byte location" && echo "✅ Passed" || echo "❌ Failed"
