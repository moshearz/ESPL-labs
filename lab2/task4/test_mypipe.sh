#!/bin/bash
echo '--- Running mypipe tests ---'
echo 'Test 1: Input: "hello"'
./mypipe "hello" > output.txt
grep -q "child process: hello" output.txt && echo '✅ Passed' || echo '❌ Failed'
echo 'Test 2: Input: "1234567890"'
./mypipe "1234567890" > output.txt
grep -q "child process: 1234567890" output.txt && echo '✅ Passed' || echo '❌ Failed'
echo 'Test 3: Input: ""'
./mypipe "" > output.txt
grep -q "child process: " output.txt && echo '✅ Passed' || echo '❌ Failed'
echo 'Test 4: Input: "!@#$%^&*()"'
./mypipe "!@#$%^&*()" > output.txt
grep -q "child process: !@#$%^&*()" output.txt && echo '✅ Passed' || echo '❌ Failed'
echo 'Test 5: Input: "This is a longer test message"'
./mypipe "This is a longer test message" > output.txt
grep -q "child process: This is a longer test message" output.txt && echo '✅ Passed' || echo '❌ Failed'
rm -f output.txt
echo '--- Tests Completed ---'
