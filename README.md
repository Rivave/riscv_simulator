# riscv_simulator

Simulador del ISA base RISC-V de 32 bits (RV32I). Mantiene el estado
arquitectural de una CPU (PC, 32 registros y memoria), carga programas
compilados en binario crudo y los ejecuta paso por paso, permitiendo
inspeccionar registros y memoria.

## Build

```sh
cmake -S . -B build
cmake --build build --config Debug
```

## Pruebas

```sh
python tests/run_tests.py
```

También se pueden ejecutar con CTest después de configurar CMake:

```sh
ctest --test-dir build -C Debug --output-on-failure
```

## Uso

```sh
./build/Debug/riscv-sim programa.bin
```

El programa se carga en la dirección `0x00000000`. La memoria es plana,
byte-addressable, con un address space de 32 bits y formato little-endian.

### Comandos del REPL

| Comando | Descripción |
| --- | --- |
| `load <archivo>` | Carga un binario crudo en `0x00000000` |
| `step [n]` | Ejecuta `n` instrucciones (por defecto 1), desensamblando cada una |
| `run` | Ejecuta hasta `ecall`/`ebreak`, límite de instrucciones o detección de ciclo de estado |
| `pc` | Muestra el contador de programa |
| `regs [r1 r2 ...]` | Muestra registros (todos si no se indican); acepta `xN`, nombres ABI y `pc` |
| `mem <inicio> <fin>` | Muestra los bytes de memoria en el rango inclusivo |
| `reset` | Reinicia PC y registros |
| `help` | Lista los comandos |
| `exit` | Sale del simulador |

## Instrucciones soportadas

ISA base RV32I (anexo A): `lb`, `lh`, `lw`, `lbu`, `lhu`, `addi`, `slli`,
`slti`, `sltiu`, `xori`, `srli`, `srai`, `ori`, `andi`, `auipc`, `sb`, `sh`,
`sw`, `add`, `sub`, `sll`, `slt`, `sltu`, `xor`, `srl`, `sra`, `or`, `and`,
`lui`, `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu`, `jalr`, `jal`.

## Extras

- `ecall` con syscalls estilo SPIM: `1` print_int, `4` print_string,
  `5` read_int, `8` read_string, `11` print_char, `12` read_char,
  `10` exit, `17` exit2.
- `read_int` descarta el salto de línea restante para que un `read_string` o
  `read_char` posterior no consuma una línea vacía accidentalmente.
- Un número de syscall no soportado se reporta como error de ejecución.
- Las syscalls de salida avanzan el PC igual que cualquier `ecall` ejecutado y
  luego marcan el simulador como finalizado.
- Desensamblador integrado que muestra cada instrucción durante `step`.

## Programas de demostración

La carpeta `programs/` contiene los `.s` originales, los `.bin` crudos y una
descripción breve de los resultados esperados. Las pruebas en `tests/` ejecutan
estos programas y validan registros, memoria y detección de ciclos.

## Entrega

El repositorio ignora `build/` y otros artefactos generados. Para preparar un
ZIP, comprime los archivos del proyecto sin incluir `.git/` ni `build/`.

## Estructura

- `include/`, `src/cpu.*`: banco de 32 registros y PC.
- `src/memory.*`: memoria dispersa little-endian con address space de 32 bits.
- `src/instruction.*`: decodificador y desensamblador RV32I.
- `src/simulator.*`: motor de ejecución (fetch / decode / execute) y `ecall`.
- `src/main.cpp`: REPL interactivo.
