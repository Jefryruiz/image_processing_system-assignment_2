#!/bin/bash
# =============================================================================
# build.sh
# Script de automatizacion completa del prototipo virtual.
#
# Pasos:
#   1. Verifica dependencias
#   2. Verifica que GEM5 esta compilado
#   3. Registra el acelerador en GEM5 y recompila si es necesario
#   4. Compila el driver bare-metal para ARM64
#   5. Genera imagen de entrada si no existe
#   6. Corre la simulacion GEM5
#   7. Verifica la imagen de salida
#
# Uso:
#   chmod +x scripts/build.sh
#   ./scripts/build.sh
# =============================================================================

set -e  # Salir si cualquier comando falla

# -----------------------------------------------------------------------------
# Colores para output
# -----------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # Sin color

info()    { echo -e "${GREEN}[INFO]${NC} $1"; }
warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

# -----------------------------------------------------------------------------
# Rutas del proyecto
# -----------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
GEM5_DIR="${HOME}/gem5"
GEM5_BIN="${GEM5_DIR}/build/ARM/gem5.opt"

DRIVER_SRC="${PROJECT_DIR}/driver/accelerator_driver.c"
DRIVER_ELF="${PROJECT_DIR}/driver/accelerator_driver.elf"
INPUT_IMG="${PROJECT_DIR}/images/input.raw"
OUTPUT_IMG="${PROJECT_DIR}/images/output.raw"
RUN_SCRIPT="${PROJECT_DIR}/scripts/run_gem5.py"

echo "============================================================"
echo " GEM5 Virtual Prototype — Build & Run Script"
echo "============================================================"

# -----------------------------------------------------------------------------
# PASO 1: Verificar dependencias
# -----------------------------------------------------------------------------
info "Paso 1: Verificando dependencias..."

command -v aarch64-linux-gnu-gcc >/dev/null 2>&1 || {
    warning "Cross-compiler no encontrado. Instalando..."
    sudo apt-get install -y gcc-aarch64-linux-gnu
}

command -v python3 >/dev/null 2>&1 || error "python3 no encontrado."

# -----------------------------------------------------------------------------
# PASO 2: Verificar GEM5
# -----------------------------------------------------------------------------
info "Paso 2: Verificando GEM5..."

if [ ! -f "$GEM5_BIN" ]; then
    error "GEM5 no compilado. Correr primero:\n  cd ~/gem5 && scons build/ARM/gem5.opt -j2"
fi

info "GEM5 encontrado en: $GEM5_BIN"

# -----------------------------------------------------------------------------
# PASO 3: Registrar acelerador en GEM5
# -----------------------------------------------------------------------------
info "Paso 3: Registrando acelerador en GEM5..."

ACCEL_PY_SRC="${PROJECT_DIR}/gem5_files/ImageAccelerator.py"
ACCEL_PY_DST="${GEM5_DIR}/src/dev/ImageAccelerator.py"
SCONSCRIPT_SRC="${PROJECT_DIR}/gem5_files/SConscript.accel"
GEM5_DEV_DIR="${GEM5_DIR}/src/dev"

# Copiar ImageAccelerator.py
if [ ! -f "$ACCEL_PY_DST" ] || ! diff -q "$ACCEL_PY_SRC" "$ACCEL_PY_DST" > /dev/null 2>&1; then
    info "Copiando ImageAccelerator.py a GEM5..."
    cp "$ACCEL_PY_SRC" "$ACCEL_PY_DST"

    # Agregar al SConscript de GEM5 si no esta ya
    if ! grep -q "ImageAccelerator" "${GEM5_DEV_DIR}/SConscript"; then
        info "Agregando ImageAccelerator al SConscript de GEM5..."
        echo "" >> "${GEM5_DEV_DIR}/SConscript"
        echo "# Image Accelerator (TLM 2.0 virtual prototype)" >> "${GEM5_DEV_DIR}/SConscript"
        echo "SimObject('ImageAccelerator.py', sim_objects=['ImageAccelerator'])" >> "${GEM5_DEV_DIR}/SConscript"
    fi

    # Copiar el header del acelerador
    cp "${PROJECT_DIR}/src/image_accelerator.h" "${GEM5_DEV_DIR}/image_accelerator.h"
    cp "${PROJECT_DIR}/src/defines.h"           "${GEM5_DEV_DIR}/defines.h"

    info "Recompilando GEM5 con el acelerador..."
    cd "$GEM5_DIR"
    scons build/ARM/gem5.opt -j2
    cd "$PROJECT_DIR"
else
    info "Acelerador ya registrado en GEM5."
fi

# -----------------------------------------------------------------------------
# PASO 4: Compilar driver bare-metal para ARM64
# -----------------------------------------------------------------------------
info "Paso 4: Compilando driver bare-metal para ARM64..."

mkdir -p "${PROJECT_DIR}/driver"

aarch64-linux-gnu-gcc \
    -static \
    -O2 \
    -march=armv8-a \
    -o "$DRIVER_ELF" \
    "$DRIVER_SRC"

info "Driver compilado: $DRIVER_ELF"

# -----------------------------------------------------------------------------
# PASO 5: Generar imagen de entrada si no existe
# -----------------------------------------------------------------------------
info "Paso 5: Verificando imagen de entrada..."

mkdir -p "${PROJECT_DIR}/images"

if [ ! -f "$INPUT_IMG" ]; then
    warning "input.raw no encontrado. Generando imagen de prueba..."
    python3 - << 'PYEOF'
import os, sys
W, H = 1920, 1080
out = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   '..', 'images', 'input.raw')
with open(out, 'wb') as f:
    for y in range(H):
        row = bytearray(W * 3)
        for x in range(W):
            row[x*3+0] = (x * 255) // W
            row[x*3+1] = (y * 255) // H
            row[x*3+2] = (x + y) % 256
        f.write(row)
print(f"Imagen generada: {os.path.abspath(out)} ({W*H*3} bytes)")
PYEOF
else
    info "Imagen de entrada encontrada: $INPUT_IMG ($(wc -c < $INPUT_IMG) bytes)"
fi

# -----------------------------------------------------------------------------
# PASO 6: Correr simulacion GEM5
# -----------------------------------------------------------------------------
info "Paso 6: Iniciando simulacion GEM5..."

mkdir -p "${PROJECT_DIR}/gem5_output"

"$GEM5_BIN" \
    --outdir="${PROJECT_DIR}/gem5_output" \
    "$RUN_SCRIPT"

# -----------------------------------------------------------------------------
# PASO 7: Verificar imagen de salida
# -----------------------------------------------------------------------------
info "Paso 7: Verificando imagen de salida..."

if [ -f "$OUTPUT_IMG" ]; then
    SIZE=$(wc -c < "$OUTPUT_IMG")
    EXPECTED=2073600
    if [ "$SIZE" -eq "$EXPECTED" ]; then
        info "Imagen de salida OK: $OUTPUT_IMG ($SIZE bytes)"
    else
        warning "Imagen de salida tiene tamano incorrecto: $SIZE (esperado: $EXPECTED)"
    fi

    # Generar preview PNG si Python y Pillow estan disponibles
    python3 -c "from PIL import Image; print('Pillow disponible')" 2>/dev/null && {
        python3 - << 'PYEOF'
from PIL import Image
import numpy as np, os
W, H = 1920, 1080
proj = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
inp  = np.fromfile(os.path.join(proj,'images','input.raw'),  dtype=np.uint8).reshape(H,W,3)
out  = np.fromfile(os.path.join(proj,'images','output.raw'), dtype=np.uint8).reshape(H,W)
Image.fromarray(inp, 'RGB').save(os.path.join(proj,'images','input_preview.png'))
Image.fromarray(out, 'L'  ).save(os.path.join(proj,'images','output_preview.png'))
print("Previews PNG generados en images/")
PYEOF
    }
else
    warning "Imagen de salida no encontrada: $OUTPUT_IMG"
    warning "Verificar logs en gem5_output/"
fi

echo "============================================================"
info "Build y simulacion completados."
echo "  Input:  $INPUT_IMG"
echo "  Output: $OUTPUT_IMG"
echo "  Logs:   ${PROJECT_DIR}/gem5_output/"
echo "============================================================"
