#!/usr/bin/env bash
# 断点重试拉取基础镜像（国内网络慢/易断时使用）
set -euo pipefail

MAX_RETRY="${MAX_RETRY:-30}"
SLEEP_SEC="${SLEEP_SEC:-10}"

pull_with_retry() {
  local target="$1"
  shift
  local spec attempt=1

  for spec in "$@"; do
    attempt=1
    while [ "$attempt" -le "$MAX_RETRY" ]; do
      echo ""
      echo "==> [$attempt/$MAX_RETRY] docker pull $spec"
      if docker pull "$spec"; then
        docker tag "$spec" "$target"
        echo "    OK: tagged as $target"
        return 0
      fi
      echo "    failed, retry in ${SLEEP_SEC}s ..."
      sleep "$SLEEP_SEC"
      attempt=$((attempt + 1))
    done
    echo "    mirror failed: $spec"
  done

  echo "ERROR: could not pull $target" >&2
  return 1
}

# 确认未配置失效代理
if docker info 2>/dev/null | grep -qi '127.0.0.1:7890'; then
  echo "WARN: Docker 仍指向 127.0.0.1:7890 代理，请先删除 proxy.conf 并 restart docker" >&2
fi

echo "== PostgreSQL base =="
pull_with_retry postgres:16-bookworm \
  docker.1ms.run/library/postgres:16-bookworm \
  docker.m.daocloud.io/library/postgres:16-bookworm \
  docker.1panel.live/library/postgres:16-bookworm

echo ""
echo "== Ubuntu base (gprom) =="
pull_with_retry ubuntu:22.04 \
  docker.1ms.run/library/ubuntu:22.04 \
  docker.m.daocloud.io/library/ubuntu:22.04 \
  docker.1panel.live/library/ubuntu:22.04

echo ""
docker images | grep -E 'postgres|ubuntu' || true
echo ""
echo "Base images ready. Build with:"
echo "  cd $(dirname "$0") && docker compose build --progress=plain postgres"
echo "  docker compose up -d postgres"
