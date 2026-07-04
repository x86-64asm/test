#!/bin/bash

echo "Testing arithmetic.ms..."
./musab-cc examples/arithmetic.ms -o /tmp/arithmetic.out
if [ $? -eq 0 ]; then
    echo "✓ Compilation successful"
else
    echo "✗ Compilation failed"
    exit 1
fi
