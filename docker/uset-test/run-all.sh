#!/usr/bin/env bash
# One-shot: postgres + smoke test + optional full three-rset experiments.
set -euo pipefail

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

RUN_FULL="${RUN_FULL:-0}"

if ! command -v docker >/dev/null 2>&1; then
  echo "ERROR: docker not found. Install first:" >&2
  echo "  sudo apt-get update && sudo apt-get install -y docker.io docker-compose-v2" >&2
  echo "  sudo usermod -aG docker \$USER && newgrp docker" >&2
  exit 1
fi

echo "== [1/3] Start PostgreSQL =="
docker compose up -d postgres
docker compose ps

echo "== [2/3] Smoke test =="
docker compose --profile test run --rm gprom-test

if [ "$RUN_FULL" = "1" ]; then
  echo "== [3/3] Full three-rset experiments =="
  docker compose --profile test run --rm -e RUN_FULL=1 gprom-test
else
  echo "== [3/3] Skip full experiments (set RUN_FULL=1 to enable) =="
fi

echo ""
echo "Done. Interactive shell:"
echo "  cd $DIR && docker compose --profile shell run --rm gprom-shell"
