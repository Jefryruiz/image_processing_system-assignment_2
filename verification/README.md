# verification/

RTL + UVM + DPI-C deliverable for **Evaluación Corta 4** (`Verificacion_Diseno_Alto_Nivel.pdf`).
Adds a real AXI4-Full RAM in SystemVerilog, verifies it with a UVM testbench,
and integrates it into the previously-built SystemC image-processing model
(`../systemc-image-processing-platform/`) via DPI-C.

## Qué se pide (resumen del PDF)

1. Describir la memoria RAM en Verilog con puerto **AXI4 Full**.
2. Crear un testbench siguiendo el estándar **UVM/SystemVerilog**.
3. Integrar esa RAM en el modelo SystemC de la evaluación anterior usando **DPI/VPI**.

## Contenido del directorio

| Ruta | Qué es |
| --- | --- |
| `rtl/axi_ram.sv` | DUT: RAM de 64 MB (`ADDR_WIDTH=26`) con slave AXI4-Full (AW/W/B + AR/R, bursts INCR, WSTRB). |
| `sv_tb/` | Entorno UVM 1.2: `axi_if`, `axi_ram_seq_item/sequencer/driver/monitor/agent`, `axi_ram_scoreboard` (checkea contra el modelo dorado DPI-C), `axi_ram_env`, secuencias/tests (`axi_ram_seq_lib.sv`, `axi_ram_test_lib.sv`), y `tb_top.sv`. |
| `dpi/ram_model.c`, `dpi/ram_model.h` | Modelo de RAM en C, único y compartido: el SystemC model lo enlaza directo (API a puntero) y el testbench UVM lo importa vía `import "DPI-C"` (API byte a byte + helpers de archivo) — es el puente DPI que conecta ambos mundos. |
| `scripts/build_all.sh` | Compila el modelo SystemC y hace un smoke-check de compilación del RTL/UVM en un solo paso. |
| `scripts/run_uvm_sim.sh` | Compila y corre el testbench UVM contra `axi_ram.sv` con Vivado (`xvlog`/`xsc`/`xelab`/`xsim`). |
| `Verificacion_Diseno_Alto_Nivel.pdf` | Enunciado original de la evaluación. |

Documentación completa (arquitectura, diagrama de bloques/secuencias,
mapa de memoria, resultados, troubleshooting) está en el **README de la raíz
del repo** (`../README.md`), sección
["AXI4 RAM, UVM Testbench and DPI-C Integration"](../README.md#axi4-ram-uvm-testbench-and-dpi-c-integration).
Este README solo cubre cómo ejecutar lo que hay en esta carpeta.

## Requisitos

- Xilinx Vivado (`xvlog`/`xsc`/`xelab`/`xsim`) en el `PATH`:
  ```bash
  source <vivado_install>/settings64.sh
  ```
- Para el paso 1 de `build_all.sh` (modelo SystemC): `g++` (C++17), `make` y una instalación de SystemC (`SYSTEMC_HOME`).

## Ejecución

### Todo en un solo paso

```bash
SYSTEMC_HOME=/ruta/a/systemc ./scripts/build_all.sh
```

Compila el modelo SystemC (`make -C ../systemc-image-processing-platform clean all`)
y luego compila + corre `axi_ram_base_test` como smoke check del RTL/UVM.
No corre las regresiones completas; para eso usar `run_uvm_sim.sh` directamente.

### Testbench UVM (`run_uvm_sim.sh`)

```bash
./scripts/run_uvm_sim.sh [test_name] [-- NOMBRE=VALOR ...]
```

| Test | Descripción |
| --- | --- |
| `axi_ram_base_test` | Solo construye el environment (chequeo de compilación). |
| `axi_ram_random_test` | Bursts de escritura/lectura aleatorios contra el modelo dorado. Plusarg: `NUM_BURSTS` (default 50). |
| `axi_ram_image_test` | Transmite una imagen RAW real por AXI4 a través del DUT y la vuelca a disco. Plusargs: `IMG_PATH`, `IMG_OUT`, `IMG_BYTES`. |

Ejemplos:

```bash
./scripts/run_uvm_sim.sh axi_ram_random_test
./scripts/run_uvm_sim.sh axi_ram_random_test -- NUM_BURSTS=200

./scripts/run_uvm_sim.sh axi_ram_image_test -- \
    IMG_PATH=../input.raw IMG_OUT=../sv_tb_output.raw IMG_BYTES=6220800
diff <(head -c 6220800 ../input.raw) ../sv_tb_output.raw && echo "identical"
```

Una corrida exitosa termina con:

```
UVM_INFO ... [SCBD_SUMMARY] bytes checked=<N> errors=0
UVM_INFO ... [SCBD_SUMMARY] *** SCOREBOARD PASSED ***
...
UVM_ERROR :    0
UVM_FATAL :    0
```

`run_uvm_sim.sh` trabaja bajo un directorio de trabajo `../sim_uvm/` (regenerado
en cada corrida); se puede borrar con `rm -rf ../sim_uvm` entre corridas.

## Troubleshooting

- **`xsc --shared` falla con
  `unknown type [0x13] section '.relr.dyn'` / `skipping incompatible ... libm.so.6` /
  `cannot find /usr/lib/x86_64-linux-gnu/libm.so.6`:** el linker embebido de
  Vivado (`tps/lnx64/binutils-2.37/bin/ld`, binutils 2.37 de 2021) no entiende
  las secciones `.relr.dyn` (relocaciones RELR) que usa el glibc de
  distribuciones Ubuntu recientes (22.04.3+/24.04+/26.04), así que descarta
  esas librerías del sistema como "incompatibles" y el link de
  `ram_model_dpi.so` falla. El `ld` del sistema sí las soporta. Arreglo:
  respaldar el `ld` embebido de Vivado y reemplazarlo por un symlink al del
  sistema (afecta solo la instalación de Vivado, no el repo; reversible
  restaurando el respaldo):

  ```bash
  sudo mv <VIVADO_HOME>/tps/lnx64/binutils-2.37/bin/ld \
          <VIVADO_HOME>/tps/lnx64/binutils-2.37/bin/ld.orig-2.37
  sudo ln -s /usr/bin/ld <VIVADO_HOME>/tps/lnx64/binutils-2.37/bin/ld
  ```

  Para revertir: `sudo rm <VIVADO_HOME>/tps/lnx64/binutils-2.37/bin/ld && sudo mv <VIVADO_HOME>/tps/lnx64/binutils-2.37/bin/ld.orig-2.37 <VIVADO_HOME>/tps/lnx64/binutils-2.37/bin/ld`.

### Modelo SystemC (referencia)

```bash
cd ../systemc-image-processing-platform
SYSTEMC_HOME=/ruta/a/systemc make clean all
./image_processor
```

Ver `../README.md` para el detalle completo del flujo CPU → RAM → acelerador → disco.
