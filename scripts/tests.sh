#!/bin/bash
output_file="tests/output.log"
make test > $output_file 2>&1

echo "Test results logged to $output_file"