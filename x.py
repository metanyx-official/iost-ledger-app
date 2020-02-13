#!/usr/bin/env python3

import os
import sys
import imp
import subprocess

speculos_dir = sys.argv[1] if len(sys.argv) > 1 else "."
subprocess.call([
    os.path.join(speculos_dir, "speculos.py"),
    "--debug",
    "-m", "nanos",
    os.path.join(speculos_dir, "apps", "btc.elf"),
    "--display", "text",
    "--apdu-port", os.environ["ADPU_PORT"]
])

#./speculos.py
#${DEBUG_MODE:-} \
#       -m ${DEVICE_MODEL} \
#       ./apps/${APP_FILE} \
#       --sdk ${SDK_VERSION} \
#       --seed "${DEVICE_SEED}" \
#       ${EXTRA_OPTIONS:-} \
#       --display text \
#       --vnc-port 41000 \
#       --apdu-port 40000

#./dive.sh
#IMAGE="nanos-dev:latest"
#SCRIPT_DIR="$(cd `dirname ${0}` ; pwd)"
#declare -a ARGS
#ARGS=()
#ARGS+=("-it" "--rm")
#ARGS+=("-v" "${SCRIPT_DIR}:${SCRIPT_DIR}" -w "$(pwd)")
#ARGS+=("-v" "/dev:/dev" "--privileged")
#ARGS+=("$IMAGE" "$@")
#docker run "${ARGS[@]}"
