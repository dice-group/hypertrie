#!/bin/bash

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."

source scripts/internal/parse_commandline_args.sh
source scripts/internal/get_conan_profile.sh

parse_commandline_args
get_conan_profile

if [ -z "${BINTRAY_API_KEY}" ]; then
  echo "BINTRAY_API_KEY is not set"
  exit 1
fi
if [ -z "${BINTRAY_USER}" ]; then
  echo "BINTRAY_USER is not set"
  exit 1
fi

conan user -p "$BINTRAY_API_KEY" -r dice-group "$BINTRAY_USER"

if [ "$4" = "development" ]; then
  hypertrie_deploy_version="dev"
elif [ "$4" = "master" ]; then
  hypertrie_deploy_version="latest"
else
  hypertrie_deploy_version=$(conan inspect . --raw version)
fi

(conan remove -f "hypertrie/$hypertrie_deploy_version@dice-group/stable" || true)
conan create . "hypertrie/$hypertrie_deploy_version@dice-group/stable" --build missing -e hypertrie_deploy_version="$hypertrie_deploy_version" --profile ${conan_profile}
conan upload "hypertrie/$hypertrie_deploy_version@dice-group/stable" --force --all -r dice-group
(conan remove -f "hypertrie/$hypertrie_deploy_version@dice-group/stable" || true)
