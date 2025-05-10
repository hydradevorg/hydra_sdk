// Mock MPFR header for WebAssembly build
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int dummy; } __mpfr_struct;
typedef __mpfr_struct mpfr_t[1];
typedef __mpfr_struct *mpfr_ptr;
typedef const __mpfr_struct *mpfr_srcptr;

typedef enum {
  MPFR_RNDN = 0,  // Round to nearest, with ties to even
  MPFR_RNDZ,      // Round toward zero
  MPFR_RNDU,      // Round toward +Inf
  MPFR_RNDD,      // Round toward -Inf
  MPFR_RNDA,      // Round away from zero
  MPFR_RNDF       // Faithful rounding
} mpfr_rnd_t;

void mpfr_init2(mpfr_ptr x, unsigned long prec);
void mpfr_clear(mpfr_ptr x);
void mpfr_set_d(mpfr_ptr rop, double op, mpfr_rnd_t rnd);
void mpfr_set(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);
int mpfr_set_str(mpfr_ptr rop, const char *s, int base, mpfr_rnd_t rnd);
double mpfr_get_d(mpfr_srcptr op, mpfr_rnd_t rnd);
unsigned long mpfr_get_prec(mpfr_srcptr x);
int mpfr_set_prec(mpfr_ptr x, unsigned long prec);
void mpfr_swap(mpfr_ptr x, mpfr_ptr y);
int mpfr_cmp(mpfr_srcptr op1, mpfr_srcptr op2);
int mpfr_snprintf(char *buf, size_t n, const char *format, ...);
size_t mpfr_get_str_ndigits(int b, unsigned long prec);

// Arithmetic functions
int mpfr_add(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd);
int mpfr_sub(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd);
int mpfr_mul(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd);
int mpfr_div(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd);

// Mathematical functions
int mpfr_sqrt(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);
int mpfr_pow(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd);
int mpfr_exp(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);
int mpfr_log(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);
int mpfr_sin(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);
int mpfr_cos(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);
int mpfr_tan(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd);

#ifdef __cplusplus
}
#endif
