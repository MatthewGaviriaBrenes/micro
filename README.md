# Compilador Micro

Compilador para el lenguaje **Micro**, desarrollado en C, que genera ensamblador
x86-64 para Linux. Basado en el capítulo 2 de *Crafting a Compiler with C*
(Fischer & LeBlanc), con dos extensiones adicionales: **constant folding**
(ejercicio 8) y **conditional expressions** (ejercicio 10).

## Requisitos

- Linux (probado en x86-64)
- `gcc` (usado tanto para compilar el proyecto como para ensamblar/enlazar
  los programas Micro generados)
- `make`

## Compilación

```bash
make
```

Esto genera el ejecutable `micro` en la raíz del proyecto.

Para limpiar binarios y archivos generados:

```bash
make clean
```

Para correr la batería de pruebas incluida:

```bash
make test
```

## Uso

```bash
./micro programa.micro
```

El compilador:

1. Analiza léxica y sintácticamente `programa.micro`.
2. Si hay errores, los reporta y termina sin generar archivos adicionales.
3. Si el programa es válido, genera `programa.s` (ensamblador x86-64) **en el
   mismo directorio** que el archivo fuente.
4. Ensambla y enlaza `programa.s` con `gcc` (vía `system()`), produciendo un
   ejecutable con el mismo nombre base (`programa`).
5. Ejecuta el programa resultante.

Ejemplo:

```bash
./micro examples/basic.micro
```

genera `examples/basic.s` y `examples/basic`, y ejecuta este último.

Solo se acepta **un argumento de línea de comandos**: la ruta al archivo
`.micro`. No hay otra forma de interactuar con el compilador.

## El lenguaje Micro

- Un único tipo de dato: `integer`.
- Identificadores implícitos (no requieren declaración previa), máximo 32
  caracteres.
- Literales enteros: secuencias de dígitos.
- Comentarios: comienzan con `--` y terminan al final de la línea.
- Palabras reservadas: `begin`, `end`, `read`, `write`.
- Toda instrucción termina en `;`.
- El programa está delimitado por `begin` ... `end`.
- Operador de asignación: `:=`.
- Operadores aritméticos: `+`, `-`.

### Ejemplo básico

```
begin
    read(A, B);
    C := A + B;
    write(C);
end
```

### Constant folding

Si ambos operandos de una expresión son literales, el valor se calcula en
tiempo de compilación y no se genera código para la operación:

```
C := 10 + 20 - 5;   -- se traduce directamente a: movl $25, C(%rip)
```

### Conditional expressions

Sintaxis: `( E1 | E2 | E3 )`. Si `E1` es distinto de cero, el valor de la
expresión es `E2`; si `E1` es cero, el valor es `E3`.

```
begin
    A := 5;
    B := (A | 100 | 200);   -- B = 100, porque A != 0
    write(B);
end
```

Pueden anidarse:

```
D := ( (A | 1 | 0) | 100 | 200 );
```

El libro asume una instrucción ficticia `Skip A` para implementar esto. Como
generamos x86-64 real, la tradujimos al patrón estándar de comparación y
salto condicional, equivalente a un
`if/else` de bajo nivel.

## Arquitectura

```
programa.micro
     |
     v
  scanner   (análisis léxico)
     |
     v
   parser   (análisis sintáctico, recursive descent)
     |
     v
 semantics  (constant folding, chequeo de identificadores)
     |
     v
symbol table
     |
     v
  codegen   (generación de x86-64)
     |
     v
programa.s
     |
     v
     gcc  (ensamblado + enlazado, vía system())
     |
     v
 ejecutable
     |
     v
  ejecución
```

### Módulos

| Módulo | Archivos | Responsabilidad |
|---|---|---|
| Scanner | `scanner.c/.h` | Tokeniza el código fuente, detecta errores léxicos |
| Parser | `parser.c/.h` | Analiza la estructura sintáctica, coordina llamadas a semántica y codegen |
| Symbol Table | `symtab.c/.h` | Registra identificadores implícitos |
| Semantics | `semantics.c/.h` | Constant folding sobre operaciones `+`/`-` |
| Codegen | `codegen.c/.h` | Genera instrucciones x86-64 |
| Main | `main.c` | Orquesta el flujo completo: parseo, generación, ensamblado, ejecución |

## Manejo de errores

- **Errores léxicos**: caracteres inválidos, identificadores demasiado
  largos (>32 caracteres).
- **Errores sintácticos**: tokens inesperados, con recuperación mediante
  sincronización hasta `;`, `end` o fin de archivo, para reportar múltiples
  errores en una sola pasada en vez de detenerse en el primero.
- En ambos casos, el compilador termina con código de salida distinto de
  cero y no genera ni deja archivos parciales.

## Pruebas

El directorio `tests/` incluye programas `.micro` que cubren:

- Casos válidos (asignaciones, expresiones, read/write, comentarios).
- Casos inválidos (falta de `;`, `begin`, `end`, paréntesis, comas).
- Constant folding.
- Conditional expressions simples y anidadas.
- Identificadores en el límite de longitud (32 caracteres) y por encima
  del límite.

## Qué viene del libro vs. qué desarrollamos nosotros

**Del libro (Fischer & LeBlanc, cap. 2):**
- Estructura general del compilador (scanner → parser → semantics → codegen).
- Gramática base de Micro y el enfoque de recursive descent parsing.
- El concepto de `current_token`, `match()`, y símbolos de acción semántica
  integrados en las rutinas del parser.
- El enunciado y la idea conceptual de constant folding (ejercicio 8) y
  conditional expressions (ejercicio 10), incluyendo la sintaxis exacta
  `( E1 | E2 | E3 )`.

**Desarrollado/adaptado por nosotros:**
- Generación de ensamblador x86-64 real (el libro no lo especifica en
  detalle para esta arquitectura).
- Traducción de la instrucción ficticia `Skip A` del ejercicio 10 a
  instrucciones x86 reales.
- Integración completa vía `main.c`: manejo de argumentos, derivación de
  paths, ensamblado/enlace/ejecución con `system()`.
- Recuperación de errores sintácticos mediante sincronización a nivel de
  statement.