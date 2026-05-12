#!/usr/bin/env bash
set -euo pipefail

echo "Task 9.4: Sequential execution of whoami and id"
echo

echo "1) whoami:"
whoami
echo

echo "2) id:"
id
echo

echo "3) Group list extracted from id:"
id -Gn | tr ' ' '\n' | awk '{print "  - " $0}'

