#!/usr/bin/env bash
set -euo pipefail

REPO_NAME="${1:-BrunoMendez/dygma-defy-engrammer}"
cd "$(dirname "$0")"

if ! gh auth status >/dev/null 2>&1; then
  echo "Logging into GitHub CLI..."
  gh auth login --hostname github.com --git-protocol https --web
fi

echo "Creating private GitHub repository: ${REPO_NAME} and pushing..."
gh repo create "${REPO_NAME}" --private --source=. --remote=origin --push

echo "Done! Private repository available at: https://github.com/${REPO_NAME}"
