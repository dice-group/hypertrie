#!/bin/bash

set -e

if [ -z ${CONAN_TOKEN+x} ]; then
    echo "CONAN_TOKEN is not set"
    exit 1
fi
if [ -z ${CONAN_USER+x} ]; then
    echo "CONAN_USER is not set"
    exit 1
fi

conan user -p $CONAN_TOKEN -r dice-group $CONAN_USER

if [ "$1" = "development" ] ; then
    hypertrie_deploy_version="dev"
elif [ "$1" = "master" ] ; then
    hypertrie_deploy_version="latest"
else
    hypertrie_deploy_version="$1"
fi

echo $hypertrie_deploy_version

(conan remove -f "hypertrie/$hypertrie_deploy_version@dice-group/stable" || true)
conan create . "hypertrie/$hypertrie_deploy_version@dice-group/stable" --build missing -e hypertrie_deploy_version="$hypertrie_deploy_version"
conan upload "hypertrie/$hypertrie_deploy_version@dice-group/stable" --force --all -r dice-group
(conan remove -f "hypertrie/$hypertrie_deploy_version@dice-group/stable" || true )
