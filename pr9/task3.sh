#!/usr/bin/env bash
set -euo pipefail

run_as_root() {
  if [[ "${EUID}" -eq 0 ]]; then
    "$@"
  else
    sudo "$@"
  fi
}

USER_NAME="$(id -un)"
USER_HOME="${HOME}"
SRC_FILE="/tmp/task9_3_source_${USER_NAME}_$$.txt"
DST_FILE="${USER_HOME}/task9_3_root_copy.txt"

echo "Task 9.3: root copies user file to user home, then user tries modify/remove"
echo

cat > "${SRC_FILE}" <<EOF
This file was created by normal user: ${USER_NAME}
Timestamp: $(date -u +"%Y-%m-%dT%H:%M:%SZ")
EOF
echo "1) Created source file as normal user: ${SRC_FILE}"

run_as_root cp "${SRC_FILE}" "${DST_FILE}"
run_as_root chown root:root "${DST_FILE}"
run_as_root chmod 0644 "${DST_FILE}"
echo "2) Copied as root to: ${DST_FILE}"
echo "   Ownership forced to root:root and mode 0644"

echo "3) Normal user tries to append data to copied file..."
if printf "User append attempt at %s\n" "$(date -u +"%Y-%m-%dT%H:%M:%SZ")" >> "${DST_FILE}" 2>/dev/null; then
  echo "   Append succeeded."
else
  echo "   Append failed (expected for root-owned file without user-write bit)."
fi

echo "4) Normal user tries to remove copied file with rm..."
if rm -f "${DST_FILE}" 2>/dev/null; then
  echo "   rm succeeded."
else
  echo "   rm failed (depends on directory permissions/sticky-bit policy)."
fi

rm -f "${SRC_FILE}"
echo "5) Source temp file removed."

