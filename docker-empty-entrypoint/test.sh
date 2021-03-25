#! /bin/sh

set -e
docker build --iidfile iid .
docker inspect $(cat iid) | jq '.[0].Config.Entrypoint'
