#!/usr/bin/env bash
set -euo pipefail

run_as_admin() {
  if [[ "${EUID}" -eq 0 ]]; then
    "$@"
    return
  fi

  if command -v sudo >/dev/null 2>&1; then
    if sudo -n true >/dev/null 2>&1; then
      sudo "$@"
    else
      echo "Sudo password may be required..."
      sudo "$@"
    fi
    return
  fi

  if command -v pkexec >/dev/null 2>&1; then
    pkexec "$@"
    return
  fi

  echo "Error: neither sudo nor pkexec is available for privilege escalation."
  return 1
}

echo "Task 9.2: cat /etc/shadow with administrative privileges"
echo

if run_as_admin cat /etc/shadow; then
  echo
  echo "Command executed successfully."
else
  echo
  echo "Command failed. Check admin-access configuration for your account."
  exit 1
fi

