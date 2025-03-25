#!/bin/bash

# Setup test environment
rm -rf /tmp/dbtest
mkdir -p /tmp/dbtest

check_failure() {
    echo "$1" | grep -q '(X)' && return 1 || return 0
}

echo ""
echo "=== TEST 1: Write/Read ==="
output1=$(./dbtest -p 5000 -S username "johndoe" 2>&1)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -G username 2>&1)
check_failure "$output2"
result2=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ]; then
    echo "✅ Test 1 Passed"
else
    echo "❌ Test 1 Failed"
fi

echo ""
echo "=== TEST 2: Delete ==="
output1=$(./dbtest -p 5000 -S tempkey "tempvalue" 2>&1)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -D tempkey 2>&1)
check_failure "$output2"
result2=$?
output3=$(./dbtest -p 5000 -G tempkey 2>&1)
check_failure "$output3"
result3=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $result3 -ne 0 ]; then
    echo "✅ Test 2 Passed"
else
    echo "❌ Test 2 Failed"
fi

echo ""
echo "=== TEST 3: Missing Key ==="
output1=$(./dbtest -p 5000 -G nonexistentkey 2>&1)
check_failure "$output1"
result1=$?
if [ $result1 -ne 0 ]; then
    echo "✅ Test 3 Passed"
else
    echo "❌ Test 3 Failed"
fi

# Cleanup
echo ""
echo "All tests completed!"