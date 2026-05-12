                                                      task6.sh *                                                                  

USER_NAME="$(id -un)"
USER_GROUP="$(id -gn)"
TEST_FILE="${HOME}/task9_5_access_test.txt"

cat > "${TEST_FILE}" <<EOF
Task 9.5 test file created by ${USER_NAME}
EOF

echo "Task 9.5: chown/chmod by root and read/write checks as normal user"
echo "Test file: ${TEST_FILE}"
echo

apply_and_test() {
  local owner="$1"
  local group="$2"
  local mode="$3"
  local label="$4"

  run_as_root chown "${owner}:${group}" "${TEST_FILE}"
  run_as_root chmod "${mode}" "${TEST_FILE}"

  local read_status="NO"
  local write_status="NO"

  if head -n 1 "${TEST_FILE}" >/dev/null 2>&1; then
    read_status="YES"
  fi

  if printf "write test (%s)\n" "${label}" >> "${TEST_FILE}" 2>/dev/null; then
    write_status="YES"
  fi

  printf "%-24s owner=%-12s mode=%-4s read=%-3s write=%-3s\n" \
    "${label}" "${owner}:${group}" "${mode}" "${read_status}" "${write_status}"
}

echo "Scenario matrix:"
apply_and_test root root 600 "root_only"
apply_and_test root root 644 "root_read_all"
apply_and_test root root 666 "world_rw"
apply_and_test "${USER_NAME}" "${USER_GROUP}" 600 "user_private"

echo
