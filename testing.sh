#!/bin/bash

# Setup test environment
make dbtest
if [ $? -ne 0 ]; then
    echo "❌ Failed to build dbtest. Exiting."
    exit 1
fi
echo "✅ Successfully built dbtest."

make dbserver
if [ $? -ne 0 ]; then
    echo "❌ Failed to build dbserver. Exiting."
    exit 1
fi
echo "✅ Successfully built dbserver."

rm -rf /tmp/dbtest
mkdir -p /tmp/dbtest

# start a server in a detached tmux session
tmux new-session -d -s dbserver "./dbserver"

# give enough time for server to start
sleep 2

# Check if the last command failed
check_failure() {
    echo "$1" | grep -q '(X)' && return 1 || return 0
}

# === TEST 1: Write/Read Operation ===
echo ""
echo "=== TEST 1: Write/Read Operations ==="
output1=$(./dbtest -p 5000 -S username "johndoe" 2>&1)
check_failure "$output1"
result1=$?
# echo "Output1: $output1"
output2=$(./dbtest -p 5000 -G username 2>&1)
check_failure "$output2"
result2=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ "$output2" = '="johndoe"' ]; then
    echo "✅ Test 1 Passed"
else
    echo "❌ Test 1 Failed"
fi

# === TEST 2: Delete Operation ===
echo ""
echo "=== TEST 2: Delete Operation ==="
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

# === TEST 3: Missing Key ===
echo ""
echo "=== TEST 3: Get Missing Key ==="
output1=$(./dbtest -p 5000 -G nonexistentkey 2>&1)
check_failure "$output1"
result1=$?
if [ $result1 -ne 0 ]; then
    echo "✅ Test 3 Passed"
else
    echo "❌ Test 3 Failed"
fi

# === TEST 4: Overwrite Key ===
echo ""
echo "=== TEST 4: Overwrite Key ==="
output1=$(./dbtest -p 5000 -S overwritekey "value1" 2>&1)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -S overwritekey "value2" 2>&1)
check_failure "$output2"
result2=$?
output3=$(./dbtest -p 5000 -G overwritekey 2>&1)
check_failure "$output3"
result3=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $result3 -eq 0 ]  && [ "$output3" = '="value2"' ]; then
    echo "✅ Test 4 Passed"
else
    echo "❌ Test 4 Failed"
fi


# === TEST 5: Delete Nonexistent Key ===
echo ""
echo "=== TEST 5: Delete Nonexistent Key ==="
output1=$(./dbtest -p 5000 -D nonexistentkey 2>&1)
check_failure "$output1"
result1=$?
if [ $result1 -ne 0 ]; then
    echo "✅ Test 5 Passed"
else
    echo "❌ Test 5 Failed"
fi

# === TEST 6: Concurrent Writes ===
echo ""
echo "=== TEST 6: Concurrent Writes ==="
output1=$(./dbtest -p 5000 -S key1 "value1" & ./dbtest -p 5000 -S key2 "value2" & wait)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -G key1 2>&1)
result2=$?
output3=$(./dbtest -p 5000 -G key2 2>&1)
result3=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $result3 -eq 0 ] && [ "$output2" = '="value1"' ] && [ "$output3" = '="value2"' ]; then
    echo "✅ Test 6 Passed"
else
    echo "❌ Test 6 Failed"
fi


# === TEST 7: Concurrent Writes On Same Key - Race Condition ===
echo ""
echo "=== TEST 7: Concurrent Writes On Same Key ==="
output1=$(./dbtest -p 5000 -S keySame "value1" & ./dbtest -p 5000 -S keySame "value2" & wait)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -G keySame 2>&1)
result2=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ]; then
    echo "✅ Test 7 Passed"
else
    echo "❌ Test 7 Failed"
fi

# === TEST 8: 6 Instructions Combination of Read, Write, and Delete ===
echo ""
echo "=== TEST 8: Combination of Read, Write, and Delete ==="
output1=$(./dbtest -p 5000 -S comboKey "initialValue" 2>&1)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -G comboKey 2>&1)
check_failure "$output2"
result2=$?
output3=$(./dbtest -p 5000 -S comboKey "updatedValue" 2>&1)
check_failure "$output3"
result3=$?
output4=$(./dbtest -p 5000 -G comboKey 2>&1)
check_failure "$output4"
result4=$?
output5=$(./dbtest -p 5000 -D comboKey 2>&1)
check_failure "$output5"
result5=$?
output6=$(./dbtest -p 5000 -G comboKey 2>&1)
check_failure "$output6"
result6=$?
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $result3 -eq 0 ] &&
   [ $result4 -eq 0 ] && [ $result5 -eq 0 ] && [ $result6 -ne 0 ]; then
    echo "✅ Test 8 Passed"
else
    echo "❌ Test 8 Failed"
fi

# === TEST 9: Write and Read 4096 Bytes of Data ===
echo ""
echo "=== TEST 9: Write and Read 4096 Bytes of Data ==="
large_value=$(head -c 4096 </dev/urandom | base64 | head -c 4096)
# generated_length=${#large_value}
# echo "Size of Generated Data: $generated_length bytes"
output1=$(./dbtest -p 5000 -S largeKey "$large_value" 2>&1)
check_failure "$output1"
result1=$?
output2=$(./dbtest -p 5000 -G largeKey 2>&1)
check_failure "$output2"
result2=$?
read_length=${#output2}
# echo "Length of Read Data: $read_length bytes"
if [ $result1 -eq 0 ] && [ $result2 -eq 0 ] && [ $read_length -eq $((4096 + 3)) ]; then # 3 bytes are added  for ="" in the output.
    echo "✅ Test 9 Passed"
else
    echo "❌ Test 9 Failed"
fi

# === TEST 10: Run dbtest with --port, --count, and --threads ===
echo ""
echo "=== TEST 10: Run dbtest with --port, --count, and --threads ==="
output=$(./dbtest --port=5000 --count=100 --threads=5 2>&1 && wait)
result=$?
if [ $result -eq 0 ] && [ -z "$output" ]; then
    echo "✅ Test 10 Passed"
else
    echo "❌ Test 10 Failed"
fi
# Cleanup
echo ""
echo "All tests completed!"
