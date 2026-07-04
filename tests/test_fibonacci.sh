#!/bin/bash

echo "Testing fibonacci.ms..."
./musab-cc examples/fibonacci.ms -o /tmp/fibonacci.out
if [ $? -eq 0 ]; then
    echo "✓ Compilation successful"
else
    echo "✗ Compilation failed"
    exit 1
fi
