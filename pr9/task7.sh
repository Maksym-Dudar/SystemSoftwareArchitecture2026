#!/usr/bin/env bash
set -euo pipefail

echo "Task 9.7: Simulating incomplete/corrupted getent passwd data"
echo

MOCK_DATA="$(mktemp)"
trap 'rm -f "${MOCK_DATA}"' EXIT

cat > "${MOCK_DATA}" <<'EOF'
validuser:x:1001:1001:Valid User:/home/validuser:/bin/bash
missing_shell:x:1002:1002:Missing Shell:/home/missingshell
bad_uid:x:not-a-number:1003:Bad UID:/home/baduid:/bin/bash
empty_home:x:1004:1004:Empty Home::/bin/bash
truncated
EOF

echo "Mock source file: ${MOCK_DATA}"
echo

awk -F: '
  function fail(msg) {
    printf "INVALID  line %d: %s | reason: %s\n", NR, $0, msg
  }
  function ok() {
    printf "VALID    line %d: %s\n", NR, $1
  }
  {
    if (NF != 7) {
      fail("expected 7 colon-separated fields")
      next
    }
    if ($3 !~ /^[0-9]+$/) {
      fail("UID is not numeric")
      next
    }
    if ($4 !~ /^[0-9]+$/) {
      fail("GID is not numeric")
      next
    }
    if ($6 == "" || $6 !~ /^\//) {
      fail("home directory is empty or not absolute")
      next
    }
    if ($7 == "") {
      fail("shell is empty")
      next
    }
    ok()
  }
' "${MOCK_DATA}"

echo
echo "Why getent passwd may return incomplete/incorrect data:"
echo "1) NSS misconfiguration in /etc/nsswitch.conf (wrong source order)."
echo "2) External identity service issues (LDAP/NIS/SSSD unreachable or timeout)."
echo "3) Corrupted or stale caches (nscd/sssd cache)."
echo "4) Broken records in external directory backend."
echo "5) Parsing/encoding problems during transport or conversion."

