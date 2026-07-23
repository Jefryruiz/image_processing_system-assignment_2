#!/bin/bash
# =============================================================================
# build.sh — Script de automatizacion completa del prototipo virtual
# =============================================================================

set -e

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; NC='\033[0m'
info()    { echo -e "${GREEN}[INFO]${NC} $1"; }
warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
GEM5_DIR="${HOME}/gem5"
GEM5_BIN="${GEM5_DIR}/build/ARM/gem5.opt"
GEM5_DEV_DIR="${GEM5_DIR}/src/dev"

DRIVER_SRC="${PROJECT_DIR}/driver/accelerator_driver.c"
DRIVER_ELF="${PROJECT_DIR}/driver/accelerator_driver.elf"
INPUT_IMG="${PROJECT_DIR}/images/input.raw"
OUTPUT_IMG="${PROJECT_DIR}/images/output.raw"
RUN_SCRIPT="${PROJECT_DIR}/scripts/run_gem5.py"

echo "============================================================"
echo " GEM5 Virtual Prototype — Build & Run Script"
echo "============================================================"

# ── PASO 1: Dependencias ──────────────────────────────────────────
info "Paso 1: Verificando dependencias..."
command -v aarch64-linux-gnu-gcc >/dev/null 2>&1 || {
    warning "Cross-compiler no encontrado. Instalando..."
    sudo apt-get install -y gcc-aarch64-linux-gnu
}
command -v python3 >/dev/null 2>&1 || error "python3 no encontrado."
info "Dependencias OK."

# ── PASO 2: Verificar GEM5 ───────────────────────────────────────
info "Paso 2: Verificando GEM5..."
[ -d "$GEM5_DIR" ] || error "GEM5 no encontrado en $GEM5_DIR"
[ -f "$GEM5_BIN" ] || error "GEM5 no compilado. Correr: cd ~/gem5 && scons build/ARM/gem5.opt -j2"
info "GEM5 OK."

# ── PASO 3: Copiar archivos del acelerador a GEM5 ────────────────
info "Paso 3: Instalando acelerador en GEM5..."

NEEDS_RECOMPILE=0

declare -A FILES_TO_COPY=(
    ["${PROJECT_DIR}/src/image_accelerator.h"]="${GEM5_DEV_DIR}/image_accelerator.h"
    ["${PROJECT_DIR}/src/image_accelerator_gem5.hh"]="${GEM5_DEV_DIR}/image_accelerator_gem5.hh"
    ["${PROJECT_DIR}/src/image_accelerator_gem5.cc"]="${GEM5_DEV_DIR}/image_accelerator_gem5.cc"
    ["${PROJECT_DIR}/src/defines.h"]="${GEM5_DEV_DIR}/defines.h"
    ["${PROJECT_DIR}/gem5_files/ImageAccelerator.py"]="${GEM5_DEV_DIR}/ImageAccelerator.py"
)

for SRC in "${!FILES_TO_COPY[@]}"; do
    DST="${FILES_TO_COPY[$SRC]}"
    BASENAME=$(basename "$SRC")
    if [ ! -f "$DST" ] || ! diff -q "$SRC" "$DST" > /dev/null 2>&1; then
        info "  Copiando $BASENAME -> $GEM5_DEV_DIR/"
        cp "$SRC" "$DST"
        NEEDS_RECOMPILE=1
    else
        info "  $BASENAME ya esta actualizado."
    fi
done

# Parchear SConscript de GEM5 (solo agrega, no reemplaza)
GEM5_SCONSCRIPT="${GEM5_DEV_DIR}/SConscript"
if ! grep -q "ImageAccelerator" "$GEM5_SCONSCRIPT"; then
    info "  Parcheando SConscript de GEM5..."
    echo "" >> "$GEM5_SCONSCRIPT"
    echo "# Image Accelerator — TLM 2.0 virtual prototype" >> "$GEM5_SCONSCRIPT"
    echo "SimObject('ImageAccelerator.py', sim_objects=['ImageAccelerator'])" >> "$GEM5_SCONSCRIPT"
    echo "Source('image_accelerator_gem5.cc')" >> "$GEM5_SCONSCRIPT"
    NEEDS_RECOMPILE=1
else
    info "  SConscript ya contiene ImageAccelerator."
fi

# ── PASO 4: Recompilar GEM5 si hubo cambios ──────────────────────
if [ "$NEEDS_RECOMPILE" -eq 1 ]; then
    info "Paso 4: Recompilando GEM5..."
    cd "$GEM5_DIR"
    scons build/ARM/gem5.opt -j$(nproc)
    cd "$PROJECT_DIR"
    info "GEM5 recompilado."
else
    info "Paso 4: Sin cambios, no se recompila GEM5."
fi

# ── PASO 5: Compilar driver ARM64 ────────────────────────────────
info "Paso 5: Compilando driver para ARM64..."
mkdir -p "${PROJECT_DIR}/driver"
aarch64-linux-gnu-gcc \
    -static -O2 -march=armv8-a \
    -o "$DRIVER_ELF" "$DRIVER_SRC"
info "Driver compilado: $DRIVER_ELF"

# ── PASO 6: Verificar imagen de entrada ──────────────────────────
info "Paso 6: Verificando imagen de entrada..."
[ -f "$INPUT_IMG" ] \
    && info "input.raw OK ($(wc -c < $INPUT_IMG) bytes)" \
    || error "input.raw no encontrado en: $INPUT_IMG\nAsegurate de colocar la imagen en images/input.raw"

# ── PASO 7: Correr simulacion ─────────────────────────────────────
info "Paso 7: Iniciando simulacion GEM5..."
mkdir -p "${PROJECT_DIR}/gem5_output"
"$GEM5_BIN" --outdir="${PROJECT_DIR}/gem5_output" "$RUN_SCRIPT"

# ── PASO 8: Verificar salida ──────────────────────────────────────
info "Paso 8: Verificando imagen de salida..."
if [ -f "$OUTPUT_IMG" ]; then
    SIZE=$(wc -c < "$OUTPUT_IMG")
    [ "$SIZE" -eq 2073600 ] \
        && info "output.raw OK ($SIZE bytes)" \
        || warning "output.raw tamano incorrecto: $SIZE (esperado: 2073600)"
else
    warning "output.raw no encontrado. Ver logs en: ${PROJECT_DIR}/gem5_output/"
fi

echo "============================================================"
info "Proceso completo."
echo "  Input:  $INPUT_IMG"
echo "  Output: $OUTPUT_IMG"
echo "  Logs:   ${PROJECT_DIR}/gem5_output/"
echo "============================================================"