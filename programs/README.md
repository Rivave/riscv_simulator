# Programas de demostración

Estos binarios son programas RV32I crudos cargados por el simulador en
`0x00000000`. Los archivos `.s` muestran el ensamblador original y los
`.bin` contienen las palabras little-endian usadas por las pruebas.

## arithmetic

Calcula `7 + 5`:

- Esperado: `x5 = 0x00000007`
- Esperado: `x6 = 0x00000005`
- Esperado: `x7 = 0x0000000C`

## memory

Guarda `0x7B` en la dirección `0x40` y la vuelve a cargar:

- Esperado: `x5 = 0x00000040`
- Esperado: `x6 = 0x0000007B`
- Esperado: `x7 = 0x0000007B`
- Esperado: `mem 0x40 0x43` muestra `7B 00 00 00`

## infinite_loop

Ejecuta `j .`. Sirve para validar la detección de ciclos de estado.

- Esperado: `run` informa `Ciclo infinito detectado`

## riscvtest

Programa de prueba de Harris & Harris (`riscvtest.s`). Ejercita `add`, `sub`,
`and`, `or`, `slt`, `addi`, `lw`, `sw`, `beq` y `jal`. El `.bin` se generó a
partir del código máquina anotado en el propio `.s`.

- Esperado: `x2 = 0x00000019` (25)
- Esperado: `mem 0x64 0x67` muestra `19 00 00 00` (escribe 25 en la dirección 100)
- Termina en un bucle infinito detectado por `run`

## quicksort

Quicksort in-place recursivo (de la guía). Coloca `{6,4,3,2,1,8,9}` en
`0x1000` y lo ordena. Ejercita `jal`/`jalr` (`call`/`ret`), pila y branches.

- Esperado: `mem 0x1000 0x101b` muestra el arreglo ordenado `{1,2,3,4,6,8,9}`
- Termina en un bucle infinito (`j _end`) detectado por `run`

