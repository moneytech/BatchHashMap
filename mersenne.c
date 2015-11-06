#define MT_N              (624)                 // length of state vector
#define MT_M              (397)                 // a period parameter
#define MT_K              (0x9908B0DFU)         // a magic constant
#define hiBit(u)       ((u) & 0x80000000U)   // mask all but highest   bit of u
#define loBit(u)       ((u) & 0x00000001U)   // mask all but lowest    bit of u
#define loBits(u)      ((u) & 0x7FFFFFFFU)   // mask     the highest   bit of u
#define mixBits(u, v)  (hiBit(u)|loBits(v))  // move hi bit of u to hi bit of v

static uint32_t   state[MT_N+1];     // state vector + 1 extra to not violate ANSI C
static uint32_t   *next;          // next random value is computed from here
static int      left = -1;      // can *next++ this many times before reloading

void seedMT(uint32_t seed)
{
    uint32_t x = (seed | 1U) & 0xFFFFFFFFU, *s = state;
    int    j;

    for(left=0, *s++=x, j=MT_N; --j;
            *s++ = (x*=69069U) & 0xFFFFFFFFU);
}


uint32_t reloadMT(void)
{
    uint32_t *p0=state, *p2=state+2, *pM=state+MT_M, s0, s1;
    int    j;

    if(left < -1)
        seedMT(4357U);

    left=MT_N-1, next=state+1;

    for(s0=state[0], s1=state[1], j=MT_N-MT_M+1; --j; s0=s1, s1=*p2++)
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? MT_K : 0U);

    for(pM=state, j=MT_M; --j; s0=s1, s1=*p2++)
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? MT_K : 0U);

    s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? MT_K : 0U);
    s1 ^= (s1 >> 11);
    s1 ^= (s1 <<  7) & 0x9D2C5680U;
    s1 ^= (s1 << 15) & 0xEFC60000U;
    return(s1 ^ (s1 >> 18));
}


uint32_t randomMT(void)
{
    uint32_t y;

    if(--left < 0)
        return(reloadMT());

    y  = *next++;
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    return(y ^ (y >> 18));
}


static uint32_t x;

// get a random 64-bit register
// Uses the "rdrand" instruction giving hardware randomness
// documentation: https://software.intel.com/en-us/articles/intel-digital-random-number-generator-drng-software-implementation-guide
static inline unsigned long rand64() {
    unsigned long r;
    __asm__ __volatile__("0:\n\t" "rdrand %0\n\t" "jnc 0b": "=r" (r) :: "cc");
    return r;
}

static inline unsigned int rand32() {
    unsigned int r;
    __asm__ __volatile__("0:\n\t" "rdrand %0\n\t" "jnc 0b": "=r" (r) :: "cc");
    return r;
}


uint32_t fastrand(void) {
#ifdef USE_GENERIC
    x = ((x * 1103515245) + 12345) & 0x7fffffff;
    return x;
#elif USE_MT
    return randomMT();
#elif USE_RAND
    return rand();
#elif USE_HARDWARE
    return rand32();
#else
    return randomMT();
#endif
}