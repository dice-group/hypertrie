#!/bin/bash

set -e

if [ -z ${BINTRAY_API_KEY+x} ]; then
    echo "BINTRAY_API_KEY is not set"
    exit 1
fi
if [ -z ${BINTRAY_USER+x} ]; then
    echo "BINTRAY_USER is not set"
    exit 1
fi

conan user -p $BINTRAY_API_KEY -r dice-group $BINTRAY_USER

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
