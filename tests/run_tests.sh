#!/bin/bash

echo "========================================"
echo "Musab Language Compiler Test Suite"
echo "========================================"
echo ""

# Make test scripts executable
chmod +x tests/test_*.sh 2>/dev/null

# Run tests
echo "[1/3] Testing hello.ms"
bash tests/test_hello.sh
echo ""

echo "[2/3] Testing arithmetic.ms"
bash tests/test_arithmetic.sh
echo ""

echo "[3/3] Testing fibonacci.ms"
bash tests/test_fibonacci.sh
echo ""

echo "========================================"
echo "Test suite complete!"
echo "========================================"
