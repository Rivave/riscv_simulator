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

