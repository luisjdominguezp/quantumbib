# quantumbib — Bitácora de Desarrollo

> **Proyecto:** PAP II · ITESO
> **Autor:** Jorge Ramón Figueroa Maya
> **Repositorio:** `Jorge-Figueroa-PAP-II` branch
> **Última actualización:** 2026-03-04

---

## Tabla de Contenidos

1. [Estado Actual](#estado-actual)
2. [Changelog por Semana](#changelog-por-semana)
3. [Detalle de Pruebas Unitarias](#detalle-de-pruebas-unitarias)
4. [Plan de Desarrollo](#plan-de-desarrollo)
5. [Decisiones Técnicas](#decisiones-técnicas)
6. [Bugs Encontrados y Corregidos](#bugs-encontrados-y-corregidos)

---

## Estado Actual

| Métrica | Valor |
|---------|-------|
| Pruebas totales | **16 / 16 pasando** |
| Semanas completadas | **5 y 6** |
| Estándar objetivo | NIST FIPS 204 (ML-DSA) |
| Set de parámetros activo | **ML-DSA-44** (NIST Level 2) |
| Multiplicación polinomial | Schoolbook O(N²) — NTT pendiente |
| Serialización de llaves/firma | Structs en crudo (bit-packing pendiente) |

---

## Changelog por Semana

### Semana 5

**Objetivo:** Integrar las bases matemáticas de ML-DSA al repo.

#### Archivos nuevos

| Archivo | Descripción |
|---------|-------------|
| `dilithium/dilithium_params.h` | Parámetros globales: N=256, q=8 380 417, D=13. Define los tres sets (ML-DSA-44/65/87) con sus constantes k, l, eta, tau, beta, gamma1, gamma2, omega. |
| `dilithium/dilithium_reduce.h` | Declaraciones de las funciones de reducción específicas para Dilithium. |
| `dilithium/dilithium_reduce.c` | Implementación de Barrett reduction (23-bit), Montgomery reduction (R=2³²) y `caddq` (conditional add q, branch-free). |

#### Archivos modificados

| Archivo | Cambio |
|---------|--------|
| `Makefile` | Se agregó `dilithium` a `SRCDIRS`. Se añadió `LIB_SRCS` (todos los módulos sin `main.c`) y el target `test` que compila y ejecuta `test_runner` con Criterion. |
| `barrett_reduction/reduction.c` | 3 bugs corregidos (ver sección de bugs). |

#### Resultado de pruebas

```
[====] Synthesis: Tested: 13 | Passing: 13 | Failing: 0 | Crashing: 0
```

---

### Semana 6

**Objetivo:** Implementar el protocolo completo de firma ML-DSA: KeyGen, Sign y Verify.

#### Archivos nuevos

| Archivo | Descripción |
|---------|-------------|
| `dilithium/dilithium_poly.h` | Define los tipos `poly`, `polyveck`, `polyvecl`, `polymat`. Declara todas las funciones de aritmética, descomposición y muestreo. |
| `dilithium/dilithium_poly.c` | Implementación completa: multiplicación schoolbook, descomposición Power2Round/HighBits/LowBits/MakeHint/UseHint, y cuatro algoritmos de muestreo SHAKE. |
| `dilithium/dilithium_keygen.h` | Define `dilithium_pk` (llave pública) y `dilithium_sk` (llave secreta). Macros `DLT_*` para ML-DSA-44. |
| `dilithium/dilithium_keygen.c` | Implementa `dilithium_keygen_from_seed` (determinístico, para pruebas) y `dilithium_keygen` (lee 32 bytes de `/dev/urandom`). |
| `dilithium/dilithium_sign.h` | Define `dilithium_sig` (c̃, z, h). API pública: `dilithium_sign` y `dilithium_verify`. |
| `dilithium/dilithium_sign.c` | Firma por muestreo de rechazo (FIPS 204 Alg. 2) y verificación (Alg. 3). |

#### Archivos modificados

| Archivo | Cambio |
|---------|--------|
| `tests/unitTests.c` | +`#include <string.h>`, +3 headers de dilithium, +3 nuevas pruebas (`keygen`, `sign_verify`, `verify_tampered`). |

#### Resultado de pruebas

```
[====] Synthesis: Tested: 16 | Passing: 16 | Failing: 0 | Crashing: 0
```

---

## Detalle de Pruebas Unitarias

### Pruebas existentes (Semanas 1–5)

| # | Suite | Nombre | Qué valida |
|---|-------|--------|------------|
| 1 | `addition` | `add_with_carry_benchmark` | Suma multi-limb de 256 bits con carry encadenado. |
| 2 | `subtraction` | `sub_with_borrow_benchmark` | Resta multi-limb con borrow propagado. |
| 3 | `multiplication` | `mult_benchmark` | Multiplicación multi-limb de 256×256 bits → 512 bits. |
| 4 | `barrett_reduction` | `barrett_reduction_benchmark` | Reducción de 256 bits módulo un primo de 256 bits usando precomputed `mu`. |
| 5 | `exponentiation` | `exponentiation_benchmark` | Exponenciación modular por cuadrados repetidos. |
| 6 | `montgomery_product` | `montgomery_product_benchmark` | Producto de Montgomery con módulo P-256. |
| 7 | `montgomery_expo` | `montgomery_expo_benchmark` | Exponenciación de Montgomery sobre P-256. |
| 8 | `squareroot` | `squareroot_benchmark` | Raíz cuadrada modular por Tonelli-Shanks. |
| 9 | `modular_inverse` | `modular_inverse_benchmark` | Inverso modular (algoritmo extendido de Euclides). |
| 10 | `check0s` | `check0s_benchmark` | Verifica si el bit menos significativo es 0. |
| 11 | `check1s` | `check1s_benchmark` | Verifica si el bit menos significativo es 1. |
| 12 | `random` | `random_benchmark` | Generación de número aleatorio multi-limb con GMP. |
| 13 | `hash` | `hash_benchmark` | SHA3-256 de un bloque de 256 bits. |

### Pruebas nuevas — Semana 6 (Dilithium ML-DSA)

---

#### `Test(dilithium, keygen)`

**Propósito:** Verificar que la generación de llaves no produce error y que la expansión SHAKE-256 funcionó.

**Entrada:** Semilla determinística de 32 bytes `{0x01, 0x02, ..., 0x20}`.

**Procedimiento:**
1. Llama a `dilithium_keygen_from_seed(&pk, &sk, seed)`.
2. Verifica que el valor de retorno sea `0` (sin error).
3. Verifica que `pk.rho` no sea un arreglo de puros ceros — garantiza que `SHAKE-256(seed ‖ k ‖ l, 128)` produjo salida real.
4. Verifica que al menos un coeficiente de `pk.t1` no sea cero — garantiza que la expansión de la matriz A y el cálculo `t = A·s1 + s2` produjeron un resultado no trivial.

**Propiedad que valida:** Corrección básica del pipeline de KeyGen (derivación de seeds → expansión de A → muestreo de s1/s2 → cálculo de t → descomposición Power2Round).

---

#### `Test(dilithium, sign_verify)`

**Propósito:** Propiedad de **completitud** — una implementación correcta debe aceptar sus propias firmas.

**Entrada:**
- Semilla `{0xAB, 0xCD, 0xEF, 0x01, ...}` (repetida 4 veces).
- Mensaje: `"PAP II – ML-DSA Week 6 test message"` (35 bytes).

**Procedimiento:**
1. Genera par de llaves con `dilithium_keygen_from_seed`.
2. Firma el mensaje con `dilithium_sign(&sig, msg, mlen, &sk)` — verifica retorno `0`.
3. Verifica la firma con `dilithium_verify(&sig, msg, mlen, &pk)` — verifica retorno `0`.

**Propiedad que valida:** El ciclo completo `KeyGen → Sign → Verify` cierra correctamente. Si cualquier paso del protocolo estuviera mal (e.g., cálculo incorrecto de `tr`, `mu`, `c̃`, o `w1`), la verificación rechazaría.

---

#### `Test(dilithium, verify_tampered)`

**Propósito:** Propiedad de **solidez** — una firma válida debe ser rechazada si el mensaje fue alterado.

**Entrada:**
- Semilla `{0xDE, 0xAD, 0xBE, 0xEF, ...}` (repetida 4 veces).
- Mensaje original: `"Authentic message for signing"` (29 bytes).

**Procedimiento:**
1. Genera par de llaves y firma el mensaje original.
2. Altera el mensaje: `msg[4] ^= 0xFF` (flip de todos los bits del 5° byte).
3. Intenta verificar la firma original contra el mensaje alterado.
4. Verifica que `dilithium_verify` retorne **distinto de 0** (rechazo).

**Por qué funciona:** Al cambiar el mensaje, el verificador recalcula `mu = SHAKE-256(tr ‖ M_alterado, 64)`, que es completamente diferente. Esto produce un `c̃'` distinto al `c̃` de la firma, por lo que `memcmp(c̃, c̃') ≠ 0` y la verificación falla correctamente.

---

## Plan de Desarrollo

### Semana 7 — NTT (Number-Theoretic Transform)

**Motivación:** La multiplicación schoolbook actual es O(N²) = O(65 536) operaciones por producto polinomial. El ciclo de firma ejecuta múltiples multiplicaciones (`A·y`, `c·s1`, `c·s2`, `c·t0`). Con NTT se reduce a O(N log N) = O(2 048).

**Tareas:**

| # | Tarea | Descripción |
|---|-------|-------------|
| 7.1 | `dilithium_ntt.h/c` | Precalcular tabla de raíces de unidad (zetas). Implementar `ntt_forward` y `ntt_inverse` en el dominio de Montgomery. |
| 7.2 | `poly_ntt_mul` | Reemplazar `poly_schoolbook_mul` por: `ntt(a)`, `ntt(b)`, pointwise mul, `ntt_inv(result)`. |
| 7.3 | Prueba de regresión | Verificar que los 16 tests siguen pasando con la nueva multiplicación. |
| 7.4 | Prueba de correctitud NTT | Probar `ntt_forward → ntt_inverse = identidad` para un polinomio conocido. |
| 7.5 | Benchmark comparativo | Medir tiempo de firma con schoolbook vs NTT (opcional). |

**Constante clave:**
```
q = 8 380 417 = 2^23 - 2^13 + 1
q ≡ 1 (mod 512) → existe raíz primitiva 2N-ésima en Z_q
Raíz primitiva 512-ésima: ζ = 1753  (por convención de referencia CRYSTALS)
```

---

### Semana 8 — Bit-packing / Serialización FIPS 204

**Motivación:** Actualmente las llaves y firmas se almacenan como structs en crudo. FIPS 204 define codificaciones compactas en bytes (BitPack, SimpleBitPack, HintBitPack) que son necesarias para interoperabilidad.

**Tareas:**

| # | Tarea | Descripción |
|---|-------|-------------|
| 8.1 | `dilithium_codec.h/c` | `pk_encode / pk_decode`: rho (32 B) + BitPack(t1, 10 bits/coeff) = 1 312 B. |
| 8.2 | `sk_encode / sk_decode` | rho + rho' + K + tr + BitPack(s1/s2, eta bits) + BitPack(t0, 13 bits). |
| 8.3 | `sig_encode / sig_decode` | c̃ (32 B) + BitPack(z, 18 bits) + HintBitPack(h, omega bits). |
| 8.4 | Pruebas de round-trip | `encode → decode` produce el struct original para pk, sk y sig. |

---

### Semana 9 — Vectores de Prueba NIST (KAT)

**Motivación:** Validar la implementación contra los Known Answer Tests oficiales de NIST FIPS 204 para ML-DSA-44.

**Tareas:**

| # | Tarea | Descripción |
|---|-------|-------------|
| 9.1 | Cargar KATs | Parsear archivo `PQCsignKAT_2528.rsp` de NIST. |
| 9.2 | `test_kat.c` | Para cada vector: semilla → keygen → sign → comparar firma byte a byte. |
| 9.3 | Corrección de desviaciones | Ajustar detalles del protocolo que difieran de los KATs. |

> **Nota:** Los KATs requieren serialización (Semana 8) y NTT correcto (Semana 7) para coincidir exactamente.

---

### Backlog / Opcionales

| Tarea | Descripción |
|-------|-------------|
| ML-DSA-65 / ML-DSA-87 | Parametrizar el código para los otros dos sets. Los structs ya tienen dimensiones `K_MAX=8, L_MAX=7`. |
| Constant-time | Eliminar branches dependientes de datos secretos en `poly_uniform_eta` y `poly_challenge`. |
| Documentación LaTeX | Reporte formal del protocolo para entrega PAP II. |

---

## Decisiones Técnicas

| Decisión | Justificación |
|----------|---------------|
| **ML-DSA-44** como set fijo | Menor overhead para desarrollo; es el set más simple (k=l=4). Los demás sets son extensión directa. |
| **Schoolbook** para multiplicación | Correcto por definición, sin complejidad de depuración. NTT es optimización futura. |
| **OpenSSL EVP XOF** para SHAKE | Ya enlazado (`-lssl -lcrypto`). Evita implementar Keccak desde cero. Se usa `EVP_DigestFinalXOF` que es la interfaz XOF estándar. |
| **Structs en crudo** (sin bit-packing) | Simplifica la implementación inicial. La serialización se agrega como módulo separado. |
| **`/dev/urandom`** para aleatoriedad | Estándar en Linux/WSL. La versión de producción requeriría `getrandom(2)` o equivalente. |
| **`dilithium_keygen_from_seed`** separado | Permite pruebas determinísticas reproducibles sin depender de `/dev/urandom`. |

---

## Bugs Encontrados y Corregidos

### Semana 5 — `barrett_reduction/reduction.c`

| # | Bug | Impacto | Fix |
|---|-----|---------|-----|
| 1 | `b_k` hardcodeado a `5`; incorrecto para `size != 4` | Resultado incorrecto para operandos de tamaño ≠ 4 | Cambiado a `size + 1` |
| 2 | `mpz_import` con `order=1` (big-endian) mientras los arreglos usan little-endian (índice 0 = LSB) | Reducción producía resultados erróneos | Cambiado a `order=-1` |
| 3 | `b_mask` inicializado con `mpz_inits` pero ausente en `mpz_clears` | Memory leak de GMP en cada llamada | Agregado `b_mask` a `mpz_clears` |

### Semana 6 — `dilithium_sign.c` / `dilithium_keygen.c`

| # | Bug | Impacto | Fix |
|---|-----|---------|-----|
| 1 | `neg_ct0` no inicializado antes de `poly_sub(&neg_ct0.vec[i], &neg_ct0.vec[i], ...)` | Lectura de memoria basura del stack → firma incorrecta | `memset(&neg_ct0, 0, sizeof(neg_ct0))` antes del loop |
| 2 | En verificación: `poly_schoolbook_mul(&ct1_scaled, &c, &t1_scaled)` aplicado *después* de `<<= D` → overflow int32_t | Coeficientes de hasta ~68 × 10⁹ >> INT32_MAX (~2.1 × 10⁹) → UB | Se precalcula `t1_scaled[j] = t1[j] << D` primero (t1 ∈ [0, 1023], resultado < q); luego se multiplica por c |
| 3 | `__builtin_alloca` en `dilithium_keygen.c` para buffer dinámico | Extensión GCC no portable; riesgo de stack overflow con buffers grandes | Reemplazado con `malloc` / `free` |
