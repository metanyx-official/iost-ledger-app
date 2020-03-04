#!/usr/bin/env python3

import os
import sys
import imp
import subprocess

app_dir = os.path.dirname(os.path.abspath(__file__))

if len(sys.argv) == 1 \
and os.path.basename(os.path.abspath(sys.argv[0])) == os.path.basename(os.path.abspath(__file__)):
    for args in (["clean"], ["all"]):
        subprocess.call([
            "env",
            "BOLOS_SDK=" + os.path.join("sdk", "nanos-secure-sdk"),
            "make"
        ] + args, cwd=app_dir)
    subprocess.call([
        sys.executable,
        os.path.join(app_dir, "sdk", "speculos", "speculos.py"),
        "--debug",
        "-m", "nanos",
        os.path.join(app_dir, "bin", "app.elf"),
        "--display", "headless",
        "--vnc-port", os.environ["VNC_PORT"],
        "--apdu-port", os.environ["ADPU_PORT"]
    ])

#os.path.join(speculos_dir, "apps", "btc.elf"),
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
