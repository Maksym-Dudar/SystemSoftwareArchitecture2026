set -euo pipefail

CURRENT_USER="$(id -un)"

UID_MIN="$(awk '/^[[:space:]]*UID_MIN[[:space:]]+/ {print $2; exit}' /etc/login.defs 2>/dev/null || true)"
if [[ -z "${UID_MIN}" || ! "${UID_MIN}" =~ ^[0-9]+$ ]]; then
  UID_MIN=1000
fi

echo "Task 9.1: Reading account data via getent passwd"
echo "Current user: ${CURRENT_USER}"
echo "Regular-user UID threshold: ${UID_MIN}"
echo

if ! command -v getent >/dev/null 2>&1; then
  echo "Error: getent is not installed. This script must be run on Linux with libc tools."
  exit 1
fi

PASSWD_DATA="$(getent passwd || true)"
if [[ -z "${PASSWD_DATA}" ]]; then
  echo "Error: getent passwd returned no data."
  exit 1
fi

echo "All discovered accounts (login:uid:home:shell):"
echo "${PASSWD_DATA}" | awk -F: '{printf "  %s:%s:%s:%s\n", $1, $3, $6, $7}'
echo

OTHER_REGULAR_USERS="$(
  echo "${PASSWD_DATA}" | awk -F: -v uid_min="${UID_MIN}" -v me="${CURRENT_USER}" '
    $3 ~ /^[0-9]+$/ && $3 >= uid_min && $1 != me {
      printf "%s:%s:%s:%s\n", $1, $3, $6, $7
    }
  '
)"

if [[ -n "${OTHER_REGULAR_USERS}" ]]; then
  echo "Regular users found besides current user:"
  echo "${OTHER_REGULAR_USERS}" | awk '{print "  " $0}'
else
  echo "No other regular users were found."
fi
