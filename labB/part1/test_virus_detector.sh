#!/bin/bash

echo "==> 🛠️ Compiling..."
make || { echo "❌ Compilation failed"; exit 1; }

echo ""
echo "==> ✅ Test 1: Load signatures and print (signatures-L)..."
echo -e "1\nsignatures-L\n2\n5\n" | ./virusDetector infected > output.txt
grep -q "Virus name: Storm" output.txt && echo "✅ Passed" || echo "❌ Failed"

echo ""
echo "==> ✅ Test 2: Detect virus in 'infected'..."
echo -e "1\nsignatures-L\n3\n5\n" | ./virusDetector infected > detect.txt
grep -q "Starting byte location: 263" detect.txt && echo "✅ Passed" || echo "❌ Failed"

echo ""
echo "==> ✅ Test 3: Fix virus in infected..."
cp infected infected_copy
echo -e "1\nsignatures-L\n4\n5\n" | ./virusDetector infected_copy
ret_byte=$(xxd -ps -s 263 -l 1 infected_copy)
if [ "$ret_byte" == "c3" ]; then
  echo "✅ Passed"
else
  echo "❌ Failed"
fi

echo ""
echo "==> ✅ Test 4: Detect virus in 'suspectfile'..."
echo -e "1\ntest-signatures.bin\n3\n5\n" | ./virusDetector suspectfile > out_suspect.txt
grep -q "Starting byte location: 4" out_suspect.txt && echo "✅ Passed" || echo "❌ Failed"

echo ""
echo "==> ✅ Test 5: Fix virus in suspectfile..."
cp suspectfile suspect_copy
echo -e "1\ntest-signatures.bin\n4\n5\n" | ./virusDetector suspect_copy
ret_byte2=$(xxd -ps -s 4 -l 1 suspect_copy)
if [ "$ret_byte2" == "c3" ]; then
  echo "✅ Passed"
else
  echo "❌ Failed"
fi
