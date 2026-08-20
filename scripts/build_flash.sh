#!/usr/bin/env bash
# Build firmware in Docker (WSL), copy bins to .pio-out/<env>, flash via esptool.exe.
#
# Usage (from Ubuntu/WSL, repo root):
#   bash ./build_flash.sh
#   bash ./build_flash.sh --env waveshare_amoled_18_v2
#   bash ./build_flash.sh --port COM6
#   bash ./build_flash.sh -i
#   bash ./build_flash.sh --flash-only
#   bash ./build_flash.sh --no-flash
#   bash ./build_flash.sh --list-envs

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
HOME_WORK="${HOME}/AthenaOS"
IMAGE="tamapetchi-pio:latest"
DEFAULT_ENV="waveshare_amoled_18"
ENV_NAME="$DEFAULT_ENV"
ESPTOOL="${REPO_ROOT}/esptool.exe"
DEFAULT_PORT="COM6"
BAUD="921600"
CHIP="esp32s3"

PORT="$DEFAULT_PORT"
INTERACTIVE=0
DO_BUILD=1
DO_FLASH=1

list_envs() {
    grep -E '^\[env:' "$REPO_ROOT/platformio.ini" | sed 's/^\[env://;s/\]$//'
}

usage() {
    cat <<EOF
Usage: $0 [options]

  --env NAME        PlatformIO env (default: ${DEFAULT_ENV})
  --list-envs       Print env names from platformio.ini
  --port COM6       Serial port (default: COM6)
  --chip esp32s3    esptool chip (default: esp32s3)
  -i, --interactive List Windows COM ports and ask
  --flash-only      Skip Docker build (use existing .pio-out/<env> bins)
  --no-flash        Build + copy only
  -h, --help        This help

Examples:
  bash ./build_flash.sh
  bash ./build_flash.sh --env waveshare_amoled_18_v2 --port COM6
  bash ./build_flash.sh -i --no-flash
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --env)
            ENV_NAME="${2:-}"
            if [[ -z "$ENV_NAME" ]]; then
                echo "ERROR: --env requires a value"
                exit 1
            fi
            shift 2
            ;;
        --list-envs)
            list_envs
            exit 0
            ;;
        --port)
            PORT="${2:-}"
            if [[ -z "$PORT" ]]; then
                echo "ERROR: --port requires a value (e.g. COM6)"
                exit 1
            fi
            shift 2
            ;;
        --chip)
            CHIP="${2:-}"
            if [[ -z "$CHIP" ]]; then
                echo "ERROR: --chip requires a value (e.g. esp32s3)"
                exit 1
            fi
            shift 2
            ;;
        -i|--interactive)
            INTERACTIVE=1
            shift
            ;;
        --flash-only)
            DO_BUILD=0
            shift
            ;;
        --no-flash)
            DO_FLASH=0
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

OUT_DIR="${REPO_ROOT}/.pio-out/${ENV_NAME}"

to_win_path() {
    if command -v wslpath >/dev/null 2>&1; then
        wslpath -w "$1"
    else
        echo "$1"
    fi
}

list_com_ports() {
    if ! command -v powershell.exe >/dev/null 2>&1; then
        return 1
    fi
    powershell.exe -NoProfile -Command \
        "[System.IO.Ports.SerialPort]::GetPortNames() | ForEach-Object { \$_ }" \
        2>/dev/null | tr -d '\r' | sed '/^$/d'
}

pick_port() {
    echo ""
    echo "Porte COM Windows:"
    local ports
    ports="$(list_com_ports || true)"
    if [[ -z "$ports" ]]; then
        echo "  (nessuna rilevata)"
    else
        echo "$ports" | sed 's/^/  /'
    fi
    echo ""
    read -r -p "Porta [${DEFAULT_PORT}]: " chosen
    chosen="$(echo "${chosen:-}" | tr -d '\r')"
    if [[ -z "$chosen" ]]; then
        PORT="$DEFAULT_PORT"
    else
        PORT="$chosen"
    fi
}

flash_cmd_human() {
    local boot part fw
    boot="$(to_win_path "$OUT_DIR/bootloader.bin")"
    part="$(to_win_path "$OUT_DIR/partitions.bin")"
    fw="$(to_win_path "$OUT_DIR/firmware.bin")"
    cat <<EOF
$(to_win_path "$ESPTOOL") --chip ${CHIP} --port ${PORT} --baud ${BAUD} write_flash -z 0x0 ${boot} 0x8000 ${part} 0x10000 ${fw}
EOF
}

do_flash() {
    if [[ ! -f "$ESPTOOL" ]]; then
        echo "ERROR: esptool.exe non trovato: $ESPTOOL"
        echo "Copia esptool.exe nella root del repo."
        return 1
    fi
    for f in bootloader.bin partitions.bin firmware.bin; do
        if [[ ! -f "$OUT_DIR/$f" ]]; then
            echo "ERROR: manca $OUT_DIR/$f"
            return 1
        fi
    done

    local boot part fw
    boot="$(to_win_path "$OUT_DIR/bootloader.bin")"
    part="$(to_win_path "$OUT_DIR/partitions.bin")"
    fw="$(to_win_path "$OUT_DIR/firmware.bin")"

    echo ""
    echo "Flash ${CHIP}  env=${ENV_NAME}  port=${PORT}  baud=${BAUD}"
    "$ESPTOOL" --chip "$CHIP" --port "$PORT" --baud "$BAUD" write_flash -z \
        0x0 "$boot" 0x8000 "$part" 0x10000 "$fw"
}

# --- build ---
if [[ "$DO_BUILD" -eq 1 ]]; then
    if ! command -v docker >/dev/null 2>&1; then
        echo "ERROR: docker non trovato"
        exit 1
    fi

    BUILD_SRC="$REPO_ROOT"
    case "$REPO_ROOT" in
        /mnt/*)
            echo "Repo su filesystem Windows, rsync -> ${HOME_WORK}"
            mkdir -p "$HOME_WORK"
            rsync -a --delete \
                --exclude '.pio' \
                --exclude '.git' \
                --exclude '.tools' \
                --exclude '.pio-out' \
                --exclude 'third_party' \
                --exclude 'xiaozhi-esp32' \
                "${REPO_ROOT}/" "${HOME_WORK}/"
            BUILD_SRC="$HOME_WORK"
            ;;
    esac

    echo "Docker build  image=${IMAGE}  env=${ENV_NAME}"
    docker run --rm --entrypoint bash \
        -v "${BUILD_SRC}:/workspace" \
        -v tamapetchi-pio-cache:/root/.platformio \
        -w /workspace \
        "$IMAGE" \
        -lc 'pip install -q -U pioarduino && pio run -e '"$ENV_NAME"
    build_rc=$?
    if [[ "$build_rc" -ne 0 ]]; then
        echo "ERROR: build fallita (exit ${build_rc})"
        exit "$build_rc"
    fi

    mkdir -p "$OUT_DIR"
    local_build="${BUILD_SRC}/.pio/build/${ENV_NAME}"
    cp -f "${local_build}/bootloader.bin" \
          "${local_build}/partitions.bin" \
          "${local_build}/firmware.bin" \
          "$OUT_DIR/"
    echo "Copiati bin in ${OUT_DIR}"
    ls -l "$OUT_DIR"/*.bin
fi

if [[ "$DO_FLASH" -eq 0 ]]; then
    echo "Skip flash (--no-flash)"
    echo "Comando flash:"
    flash_cmd_human
    exit 0
fi

if [[ "$INTERACTIVE" -eq 1 ]]; then
    if [[ ! -t 0 ]]; then
        echo "ERROR: -i richiede un terminale interattivo"
        exit 1
    fi
    pick_port
fi

if ! do_flash; then
    echo ""
    echo "Flash FALLITO. Spesso il chip non e' in download mode."
    echo "  1) Tieni premuto BOOT, tocca RESET, rilascia BOOT"
    echo "  2) Riprova il comando:"
    echo ""
    flash_cmd_human
    echo ""
    if [[ -t 0 ]]; then
        read -r -p "Riprovare ora? [y/N] " retry
        retry="$(echo "${retry:-}" | tr -d '\r')"
        if [[ "$retry" == "y" || "$retry" == "Y" ]]; then
            if [[ "$INTERACTIVE" -eq 1 ]]; then
                pick_port
            fi
            if do_flash; then
                echo "Flash OK"
                exit 0
            fi
            echo ""
            echo "Ancora fallito. Comando:"
            flash_cmd_human
            exit 1
        fi
    fi
    exit 1
fi

echo "Flash OK"
exit 0
