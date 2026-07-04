#!/bin/bash

echo "Testing hello.ms..."
./musab-cc examples/hello.ms -o /tmp/hello.out
if [ $? -eq 0 ]; then
    echo "✓ Compilation successful"
else
    echo "✗ Compilation failed"
    exit 1
fi
