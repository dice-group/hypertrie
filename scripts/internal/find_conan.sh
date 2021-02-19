#!/bin/bash

function find_conan() {
  if ! type conan; then
    if [ -f ~/.local/bin/conan ]; then
      export PATH="${HOME}/.local/bin:${PATH}"
    else
      echo "Could not find conan."
      exit 1
    fi
  fi
}
