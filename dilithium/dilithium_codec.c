#include <string.h>
#include "dilithium_codec.h"
#include "dilithium_params.h"

/* polyt1: 10 bits por coeficiente, 4 coefs en 5 bytes */
static void polyt1_pack(uint8_t *r, const poly *a)
{
    unsigned int i;
    for (i = 0; i < DILITHIUM_N / 4; i++) {
        r[5*i+0] =  (uint8_t)(a->coeffs[4*i+0]);
        r[5*i+1] =  (uint8_t)(a->coeffs[4*i+0] >> 8) | (uint8_t)(a->coeffs[4*i+1] << 2);
        r[5*i+2] =  (uint8_t)(a->coeffs[4*i+1] >> 6) | (uint8_t)(a->coeffs[4*i+2] << 4);
        r[5*i+3] =  (uint8_t)(a->coeffs[4*i+2] >> 4) | (uint8_t)(a->coeffs[4*i+3] << 6);
        r[5*i+4] =  (uint8_t)(a->coeffs[4*i+3] >> 2);
    }
}

static void polyt1_unpack(poly *r, const uint8_t *a)
{
    unsigned int i;
    for (i = 0; i < DILITHIUM_N / 4; i++) {
        r->coeffs[4*i+0] = ((uint32_t)a[5*i+0]      | ((uint32_t)a[5*i+1] << 8)) & 0x3ff;
        r->coeffs[4*i+1] = ((uint32_t)a[5*i+1] >> 2 | ((uint32_t)a[5*i+2] << 6)) & 0x3ff;
        r->coeffs[4*i+2] = ((uint32_t)a[5*i+2] >> 4 | ((uint32_t)a[5*i+3] << 4)) & 0x3ff;
        r->coeffs[4*i+3] = ((uint32_t)a[5*i+3] >> 6 | ((uint32_t)a[5*i+4] << 2)) & 0x3ff;
    }
}

/* polyeta: 3 bits por coeficiente (eta=2), 8 coefs en 3 bytes, offset ETA-coef */
static void polyeta_pack(uint8_t *r, const poly *a)
{
    unsigned int i;
    uint8_t t[8];

    for (i = 0; i < DILITHIUM_N / 8; i++) {
        t[0] = (uint8_t)(DLT_ETA - a->coeffs[8*i+0]);
        t[1] = (uint8_t)(DLT_ETA - a->coeffs[8*i+1]);
        t[2] = (uint8_t)(DLT_ETA - a->coeffs[8*i+2]);
        t[3] = (uint8_t)(DLT_ETA - a->coeffs[8*i+3]);
        t[4] = (uint8_t)(DLT_ETA - a->coeffs[8*i+4]);
        t[5] = (uint8_t)(DLT_ETA - a->coeffs[8*i+5]);
        t[6] = (uint8_t)(DLT_ETA - a->coeffs[8*i+6]);
        t[7] = (uint8_t)(DLT_ETA - a->coeffs[8*i+7]);

        r[3*i+0]  = (t[0])     | (t[1] << 3) | (t[2] << 6);
        r[3*i+1]  = (t[2] >> 2)| (t[3] << 1) | (t[4] << 4) | (t[5] << 7);
        r[3*i+2]  = (t[5] >> 1)| (t[6] << 2) | (t[7] << 5);
    }
}

static void polyeta_unpack(poly *r, const uint8_t *a)
{
    unsigned int i;

    for (i = 0; i < DILITHIUM_N / 8; i++) {
        r->coeffs[8*i+0] =  (a[3*i+0])       & 7;
        r->coeffs[8*i+1] =  (a[3*i+0] >> 3)  & 7;
        r->coeffs[8*i+2] = ((a[3*i+0] >> 6) | (a[3*i+1] << 2)) & 7;
        r->coeffs[8*i+3] =  (a[3*i+1] >> 1)  & 7;
        r->coeffs[8*i+4] =  (a[3*i+1] >> 4)  & 7;
        r->coeffs[8*i+5] = ((a[3*i+1] >> 7) | (a[3*i+2] << 1)) & 7;
        r->coeffs[8*i+6] =  (a[3*i+2] >> 2)  & 7;
        r->coeffs[8*i+7] =  (a[3*i+2] >> 5)  & 7;

        r->coeffs[8*i+0] = DLT_ETA - r->coeffs[8*i+0];
        r->coeffs[8*i+1] = DLT_ETA - r->coeffs[8*i+1];
        r->coeffs[8*i+2] = DLT_ETA - r->coeffs[8*i+2];
        r->coeffs[8*i+3] = DLT_ETA - r->coeffs[8*i+3];
        r->coeffs[8*i+4] = DLT_ETA - r->coeffs[8*i+4];
        r->coeffs[8*i+5] = DLT_ETA - r->coeffs[8*i+5];
        r->coeffs[8*i+6] = DLT_ETA - r->coeffs[8*i+6];
        r->coeffs[8*i+7] = DLT_ETA - r->coeffs[8*i+7];
    }
}

/* polyt0: 13 bits por coeficiente, 8 coefs en 13 bytes, offset (1<<12)-coef */
static void polyt0_pack(uint8_t *r, const poly *a)
{
    unsigned int i;
    uint32_t t[8];

    for (i = 0; i < DILITHIUM_N / 8; i++) {
        t[0] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+0]);
        t[1] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+1]);
        t[2] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+2]);
        t[3] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+3]);
        t[4] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+4]);
        t[5] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+5]);
        t[6] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+6]);
        t[7] = (uint32_t)((1 << (DILITHIUM_D-1)) - a->coeffs[8*i+7]);

        r[13*i+ 0]  =  (uint8_t) t[0];
        r[13*i+ 1]  =  (uint8_t)(t[0] >>  8);
        r[13*i+ 1] |=  (uint8_t)(t[1] <<  5);
        r[13*i+ 2]  =  (uint8_t)(t[1] >>  3);
        r[13*i+ 3]  =  (uint8_t)(t[1] >> 11);
        r[13*i+ 3] |=  (uint8_t)(t[2] <<  2);
        r[13*i+ 4]  =  (uint8_t)(t[2] >>  6);
        r[13*i+ 4] |=  (uint8_t)(t[3] <<  7);
        r[13*i+ 5]  =  (uint8_t)(t[3] >>  1);
        r[13*i+ 6]  =  (uint8_t)(t[3] >>  9);
        r[13*i+ 6] |=  (uint8_t)(t[4] <<  4);
        r[13*i+ 7]  =  (uint8_t)(t[4] >>  4);
        r[13*i+ 8]  =  (uint8_t)(t[4] >> 12);
        r[13*i+ 8] |=  (uint8_t)(t[5] <<  1);
        r[13*i+ 9]  =  (uint8_t)(t[5] >>  7);
        r[13*i+ 9] |=  (uint8_t)(t[6] <<  6);
        r[13*i+10]  =  (uint8_t)(t[6] >>  2);
        r[13*i+11]  =  (uint8_t)(t[6] >> 10);
        r[13*i+11] |=  (uint8_t)(t[7] <<  3);
        r[13*i+12]  =  (uint8_t)(t[7] >>  5);
    }
}

static void polyt0_unpack(poly *r, const uint8_t *a)
{
    unsigned int i;

    for (i = 0; i < DILITHIUM_N / 8; i++) {
        r->coeffs[8*i+0]  =  (uint32_t)a[13*i+0];
        r->coeffs[8*i+0] |=  (uint32_t)a[13*i+1] << 8;
        r->coeffs[8*i+0] &= 0x1fff;

        r->coeffs[8*i+1]  =  (uint32_t)a[13*i+1] >> 5;
        r->coeffs[8*i+1] |=  (uint32_t)a[13*i+2] << 3;
        r->coeffs[8*i+1] |=  (uint32_t)a[13*i+3] << 11;
        r->coeffs[8*i+1] &= 0x1fff;

        r->coeffs[8*i+2]  =  (uint32_t)a[13*i+3] >> 2;
        r->coeffs[8*i+2] |=  (uint32_t)a[13*i+4] << 6;
        r->coeffs[8*i+2] &= 0x1fff;

        r->coeffs[8*i+3]  =  (uint32_t)a[13*i+4] >> 7;
        r->coeffs[8*i+3] |=  (uint32_t)a[13*i+5] << 1;
        r->coeffs[8*i+3] |=  (uint32_t)a[13*i+6] << 9;
        r->coeffs[8*i+3] &= 0x1fff;

        r->coeffs[8*i+4]  =  (uint32_t)a[13*i+6] >> 4;
        r->coeffs[8*i+4] |=  (uint32_t)a[13*i+7] << 4;
        r->coeffs[8*i+4] |=  (uint32_t)a[13*i+8] << 12;
        r->coeffs[8*i+4] &= 0x1fff;

        r->coeffs[8*i+5]  =  (uint32_t)a[13*i+8] >> 1;
        r->coeffs[8*i+5] |=  (uint32_t)a[13*i+9] << 7;
        r->coeffs[8*i+5] &= 0x1fff;

        r->coeffs[8*i+6]  =  (uint32_t)a[13*i+ 9] >> 6;
        r->coeffs[8*i+6] |=  (uint32_t)a[13*i+10] << 2;
        r->coeffs[8*i+6] |=  (uint32_t)a[13*i+11] << 10;
        r->coeffs[8*i+6] &= 0x1fff;

        r->coeffs[8*i+7]  =  (uint32_t)a[13*i+11] >> 3;
        r->coeffs[8*i+7] |=  (uint32_t)a[13*i+12] << 5;
        r->coeffs[8*i+7] &= 0x1fff;

        r->coeffs[8*i+0] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+0];
        r->coeffs[8*i+1] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+1];
        r->coeffs[8*i+2] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+2];
        r->coeffs[8*i+3] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+3];
        r->coeffs[8*i+4] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+4];
        r->coeffs[8*i+5] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+5];
        r->coeffs[8*i+6] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+6];
        r->coeffs[8*i+7] = (1 << (DILITHIUM_D-1)) - (int32_t)r->coeffs[8*i+7];
    }
}

/* polyz: 18 bits por coeficiente (gamma1=2^17), 4 coefs en 9 bytes */
static void polyz_pack(uint8_t *r, const poly *a)
{
    unsigned int i;
    uint32_t t[4];

    for (i = 0; i < DILITHIUM_N / 4; i++) {
        t[0] = (uint32_t)(DLT_GAMMA1 - a->coeffs[4*i+0]);
        t[1] = (uint32_t)(DLT_GAMMA1 - a->coeffs[4*i+1]);
        t[2] = (uint32_t)(DLT_GAMMA1 - a->coeffs[4*i+2]);
        t[3] = (uint32_t)(DLT_GAMMA1 - a->coeffs[4*i+3]);

        r[9*i+0]  = (uint8_t) t[0];
        r[9*i+1]  = (uint8_t)(t[0] >>  8);
        r[9*i+2]  = (uint8_t)(t[0] >> 16);
        r[9*i+2] |= (uint8_t)(t[1] <<  2);
        r[9*i+3]  = (uint8_t)(t[1] >>  6);
        r[9*i+4]  = (uint8_t)(t[1] >> 14);
        r[9*i+4] |= (uint8_t)(t[2] <<  4);
        r[9*i+5]  = (uint8_t)(t[2] >>  4);
        r[9*i+6]  = (uint8_t)(t[2] >> 12);
        r[9*i+6] |= (uint8_t)(t[3] <<  6);
        r[9*i+7]  = (uint8_t)(t[3] >>  2);
        r[9*i+8]  = (uint8_t)(t[3] >> 10);
    }
}

static void polyz_unpack(poly *r, const uint8_t *a)
{
    unsigned int i;

    for (i = 0; i < DILITHIUM_N / 4; i++) {
        r->coeffs[4*i+0]  =  (uint32_t)a[9*i+0];
        r->coeffs[4*i+0] |=  (uint32_t)a[9*i+1] << 8;
        r->coeffs[4*i+0] |=  (uint32_t)a[9*i+2] << 16;
        r->coeffs[4*i+0] &= 0x3ffff;

        r->coeffs[4*i+1]  =  (uint32_t)a[9*i+2] >> 2;
        r->coeffs[4*i+1] |=  (uint32_t)a[9*i+3] << 6;
        r->coeffs[4*i+1] |=  (uint32_t)a[9*i+4] << 14;
        r->coeffs[4*i+1] &= 0x3ffff;

        r->coeffs[4*i+2]  =  (uint32_t)a[9*i+4] >> 4;
        r->coeffs[4*i+2] |=  (uint32_t)a[9*i+5] << 4;
        r->coeffs[4*i+2] |=  (uint32_t)a[9*i+6] << 12;
        r->coeffs[4*i+2] &= 0x3ffff;

        r->coeffs[4*i+3]  =  (uint32_t)a[9*i+6] >> 6;
        r->coeffs[4*i+3] |=  (uint32_t)a[9*i+7] << 2;
        r->coeffs[4*i+3] |=  (uint32_t)a[9*i+8] << 10;
        r->coeffs[4*i+3] &= 0x3ffff;

        r->coeffs[4*i+0] = DLT_GAMMA1 - (int32_t)r->coeffs[4*i+0];
        r->coeffs[4*i+1] = DLT_GAMMA1 - (int32_t)r->coeffs[4*i+1];
        r->coeffs[4*i+2] = DLT_GAMMA1 - (int32_t)r->coeffs[4*i+2];
        r->coeffs[4*i+3] = DLT_GAMMA1 - (int32_t)r->coeffs[4*i+3];
    }
}

/* polyw1: 6 bits por coeficiente, w1 in [0,43], 4 coefs en 3 bytes */
static void polyw1_pack(uint8_t *r, const poly *a)
{
    unsigned int i;
    for (i = 0; i < DILITHIUM_N / 4; i++) {
        r[3*i+0]  = (uint8_t) a->coeffs[4*i+0];
        r[3*i+0] |= (uint8_t)(a->coeffs[4*i+1] << 6);
        r[3*i+1]  = (uint8_t)(a->coeffs[4*i+1] >> 2);
        r[3*i+1] |= (uint8_t)(a->coeffs[4*i+2] << 4);
        r[3*i+2]  = (uint8_t)(a->coeffs[4*i+2] >> 4);
        r[3*i+2] |= (uint8_t)(a->coeffs[4*i+3] << 2);
    }
}

/* w1Encode — Alg. 28 FIPS 204 */
void w1_encode(uint8_t *out, const polyveck *w1)
{
    unsigned int i;
    for (i = 0; i < DLT_K; i++)
        polyw1_pack(out + i * POLYW1_PACKEDBYTES, &w1->vec[i]);
}

/* pkEncode / pkDecode — Alg. 22/23 FIPS 204 */
void pk_encode(uint8_t pk[MLDSA44_PK_BYTES],
               const uint8_t rho[DILITHIUM_SEEDBYTES],
               const polyveck *t1)
{
    unsigned int i;
    memcpy(pk, rho, DILITHIUM_SEEDBYTES);
    pk += DILITHIUM_SEEDBYTES;
    for (i = 0; i < DLT_K; i++)
        polyt1_pack(pk + i * POLYT1_PACKEDBYTES, &t1->vec[i]);
}

void pk_decode(uint8_t rho[DILITHIUM_SEEDBYTES],
               polyveck *t1,
               const uint8_t pk[MLDSA44_PK_BYTES])
{
    unsigned int i;
    memcpy(rho, pk, DILITHIUM_SEEDBYTES);
    pk += DILITHIUM_SEEDBYTES;
    for (i = 0; i < DLT_K; i++)
        polyt1_unpack(&t1->vec[i], pk + i * POLYT1_PACKEDBYTES);
}

/* skEncode / skDecode — Alg. 24/25 FIPS 204 */
void sk_encode(uint8_t sk[MLDSA44_SK_BYTES],
               const uint8_t rho[DILITHIUM_SEEDBYTES],
               const uint8_t K[DILITHIUM_SEEDBYTES],
               const uint8_t tr[DILITHIUM_TRBYTES],
               const polyvecl *s1,
               const polyveck *s2,
               const polyveck *t0)
{
    unsigned int i;

    memcpy(sk, rho, DILITHIUM_SEEDBYTES);  sk += DILITHIUM_SEEDBYTES;
    memcpy(sk, K,   DILITHIUM_SEEDBYTES);  sk += DILITHIUM_SEEDBYTES;
    memcpy(sk, tr,  DILITHIUM_TRBYTES);    sk += DILITHIUM_TRBYTES;

    for (i = 0; i < DLT_L; i++)
        polyeta_pack(sk + i * POLYETA_PACKEDBYTES, &s1->vec[i]);
    sk += DLT_L * POLYETA_PACKEDBYTES;

    for (i = 0; i < DLT_K; i++)
        polyeta_pack(sk + i * POLYETA_PACKEDBYTES, &s2->vec[i]);
    sk += DLT_K * POLYETA_PACKEDBYTES;

    for (i = 0; i < DLT_K; i++)
        polyt0_pack(sk + i * POLYT0_PACKEDBYTES, &t0->vec[i]);
}

void sk_decode(uint8_t rho[DILITHIUM_SEEDBYTES],
               uint8_t K[DILITHIUM_SEEDBYTES],
               uint8_t tr[DILITHIUM_TRBYTES],
               polyvecl *s1,
               polyveck *s2,
               polyveck *t0,
               const uint8_t sk[MLDSA44_SK_BYTES])
{
    unsigned int i;

    memcpy(rho, sk, DILITHIUM_SEEDBYTES);  sk += DILITHIUM_SEEDBYTES;
    memcpy(K,   sk, DILITHIUM_SEEDBYTES);  sk += DILITHIUM_SEEDBYTES;
    memcpy(tr,  sk, DILITHIUM_TRBYTES);    sk += DILITHIUM_TRBYTES;

    for (i = 0; i < DLT_L; i++)
        polyeta_unpack(&s1->vec[i], sk + i * POLYETA_PACKEDBYTES);
    sk += DLT_L * POLYETA_PACKEDBYTES;

    for (i = 0; i < DLT_K; i++)
        polyeta_unpack(&s2->vec[i], sk + i * POLYETA_PACKEDBYTES);
    sk += DLT_K * POLYETA_PACKEDBYTES;

    for (i = 0; i < DLT_K; i++)
        polyt0_unpack(&t0->vec[i], sk + i * POLYT0_PACKEDBYTES);
}

/* sigEncode / sigDecode — Alg. 26/27 FIPS 204 */
void sig_encode(uint8_t sig[MLDSA44_SIG_BYTES],
                const dilithium_sig *s)
{
    unsigned int i, j, idx;

    memcpy(sig, s->c_tilde, DLT_CTILDEBYTES);
    sig += DLT_CTILDEBYTES;

    for (i = 0; i < DLT_L; i++)
        polyz_pack(sig + i * POLYZ_PACKEDBYTES, &s->z.vec[i]);
    sig += DLT_L * POLYZ_PACKEDBYTES;

    /* HintBitPack: omega bytes con indices de los 1s, luego k bytes de offset */
    memset(sig, 0, DLT_OMEGA + DLT_K);
    idx = 0;
    for (i = 0; i < DLT_K; i++) {
        for (j = 0; j < DILITHIUM_N; j++) {
            if (s->h.vec[i].coeffs[j] != 0)
                sig[idx++] = (uint8_t)j;
        }
        sig[DLT_OMEGA + i] = (uint8_t)idx;
    }
}

int sig_decode(dilithium_sig *s,
               const uint8_t sig[MLDSA44_SIG_BYTES])
{
    unsigned int i, j, idx;

    memcpy(s->c_tilde, sig, DLT_CTILDEBYTES);
    sig += DLT_CTILDEBYTES;

    for (i = 0; i < DLT_L; i++)
        polyz_unpack(&s->z.vec[i], sig + i * POLYZ_PACKEDBYTES);
    sig += DLT_L * POLYZ_PACKEDBYTES;

    /* HintBitUnpack — valida formato */
    idx = 0;
    for (i = 0; i < DLT_K; i++) {
        unsigned int k;
        for (k = 0; k < DILITHIUM_N; k++)
            s->h.vec[i].coeffs[k] = 0;

        if (sig[DLT_OMEGA + i] < idx || sig[DLT_OMEGA + i] > DLT_OMEGA)
            return -1;

        for (j = idx; j < sig[DLT_OMEGA + i]; j++) {
            if (j > idx && sig[j] <= sig[j-1])
                return -1;
            s->h.vec[i].coeffs[sig[j]] = 1;
        }
        idx = sig[DLT_OMEGA + i];
    }

    for (j = idx; j < DLT_OMEGA; j++) {
        if (sig[j])
            return -1;
    }

    return 0;
}
