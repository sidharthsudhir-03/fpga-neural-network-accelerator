// note: only **one** of the options below should be 1
// if multiples are set to 1, the topmost takes priority
// if all are set to 0, no acceleration is used (pure software)
#define ACCEL_FULL_LAYER      0  // dot product + bias + optional ReLU in hardware
#define ACCEL_CACHED_DOT     1  // dot product hardware with on-chip SRAM activation reuse
#define ACCEL_DOT_ONLY       0  // dot product hardware using SDRAM
#define ACCEL_MEMCOPY_ONLY   0  // memory transfer accelerator only

volatile unsigned* hex = (volatile unsigned*)0x00001010; /* hex display PIO */
volatile unsigned* wordcopy_acc = (volatile unsigned*)0x00001040; /* memory copy accelerator */
volatile unsigned* dotprod_acc = (volatile unsigned*)0x00001100; /* dot product accelerator */
volatile unsigned* act_acc = (volatile unsigned*)0x00001200; /* dot product + activation function accelerator */
volatile      int* vga = (volatile      int*)0x00004000; /* VGA adapter base address */
volatile      int* bank0 = (volatile      int*)0x00006000; /* SRAM bank0 */
volatile      int* bank1 = (volatile      int*)0x00007000; /* SRAM bank1 */

/* normally these would be contiguous but it is useful to know where they are for debugging */
volatile int* nn = (volatile int*)0x08000000; /* neural network biases and weights */
volatile int* input = (volatile int*)0x08800000; /* input image */
volatile int* l1_acts = (volatile int*)0x08801000; /* activations of layer 1 */
volatile int* l2_acts = (volatile int*)0x08802000; /* activations of layer 2 */
volatile int* l3_acts = (volatile int*)0x08803000; /* activations of layer 3 / outputs */

#include "vga_plot.c"

#define L1_IN  784
#define L1_OUT 1000
#define L2_IN  L1_OUT
#define L2_OUT 1000
#define L3_IN  L2_OUT
#define L3_OUT 10

#define NPARAMS (L1_OUT + L1_IN * L1_OUT + L2_OUT + L2_IN * L2_OUT + L3_OUT + L3_IN * L3_OUT)

int hex7seg(unsigned d)
{
    const unsigned digits[] = {
        0x40, 0x79, 0x24, 0x30, 0x19,
        0x12, 0x02, 0x78, 0x00, 0x10
    };

    return (d < 10) ? digits[d] : 0x3f;
}

/* ----------------------------------------------------------------
 * Memory copy functions
 */

 /* Hardware memory-transfer engine; pointers must be word-aligned. */
void wordcopy_hw(volatile int* dst, volatile int* src, int n_words)
{
    *(wordcopy_acc + 1) = (unsigned)dst;
    *(wordcopy_acc + 2) = (unsigned)src;
    *(wordcopy_acc + 3) = (unsigned)n_words;

    *wordcopy_acc = 0; /* start */
    *wordcopy_acc;     /* wait until the accelerator is finished */
}

/* Software reference memory copy. */
void wordcopy_sw(volatile int* dst, volatile int* src, int n_words)
{
    for (int i = 0; i < n_words; i++) {
        dst[i] = src[i];
    }
}

void wordcopy(volatile int* dst, volatile int* src, int n_words)
{
#if (ACCEL_MEMCOPY_ONLY || ACCEL_DOT_ONLY || ACCEL_CACHED_DOT || ACCEL_FULL_LAYER)
    wordcopy_hw(dst, src, n_words);
#else
    wordcopy_sw(dst, src, n_words);
#endif
}

/* ----------------------------------------------------------------
 * Dot product functions
 */

 /*
  * Hardware dot-product accelerator.
  *
  * ACCEL_DOT_ONLY uses external SDRAM.
  * ACCEL_CACHED_DOT uses on-chip SRAM banks for activation reuse.
  */
int dotprod_hw(int n_in, volatile int* w, volatile int* ifmap)
{
    *(dotprod_acc + 2) = (unsigned)w;
    *(dotprod_acc + 3) = (unsigned)ifmap;
    *(dotprod_acc + 5) = (unsigned)n_in;

    *dotprod_acc = 0;  /* start */
    return *dotprod_acc; /* wait until the accelerator is finished and return result */
}

/* Software reference dot-product implementation. */
int dotprod_sw(int n_in, volatile int* w, volatile int* ifmap)
{
    int sum = 0;

    for (unsigned i = 0; i < n_in; ++i) {
        /* Q16.16 fixed-point dot product */
        sum += (int)(((long long)w[i] * (long long)ifmap[i]) >> 16);
    }

    return sum;
}

/*
 * Computes one neural-network layer using either hardware-accelerated
 * or software-based dot products.
 */
void apply_layer_dot(
    int n_in,
    int n_out,
    volatile int* b,
    volatile int* w,
    int use_relu,
    volatile int* ifmap,
    volatile int* ofmap
)
{
    for (unsigned o = 0, wo = 0; o < n_out; ++o, wo += n_in) {
        int sum = b[o];

#if (ACCEL_DOT_ONLY || ACCEL_CACHED_DOT)
        sum += dotprod_hw(n_in, &w[wo], ifmap);
#else
        sum += dotprod_sw(n_in, &w[wo], ifmap);
#endif

        if (use_relu) {
            sum = (sum < 0) ? 0 : sum;
        }

        ofmap[o] = sum;
    }
}

/*
 * Full-layer hardware accelerator:
 * dot product + bias + optional ReLU in hardware.
 */
void apply_layer_act(
    int n_in,
    int n_out,
    volatile int* b,
    volatile int* w,
    int use_relu,
    volatile int* ifmap,
    volatile int* ofmap
)
{
    *(act_acc + 3) = (unsigned)ifmap;
    *(act_acc + 5) = (unsigned)n_in;
    *(act_acc + 7) = (unsigned)use_relu;

    for (unsigned o = 0, wo = 0; o < n_out; ++o, wo += n_in) {
        *(act_acc + 1) = (unsigned)(b + o);
        *(act_acc + 2) = (unsigned)(w + wo);
        *(act_acc + 4) = (unsigned)(ofmap + o);

        *act_acc = 0; /* start */
    }

    *act_acc; /* wait until the accelerator is finished */
}

/* ----------------------------------------------------------------
 * Utility functions
 */

int max_index(int n_in, volatile int* ifmap)
{
    int max_sofar = 0;

    for (int i = 1; i < n_in; ++i) {
        if (ifmap[i] > ifmap[max_sofar]) {
            max_sofar = i;
        }
    }

    return max_sofar;
}

void display_image(
    volatile int* image,
    int rows,
    int cols,
    int min_pixel_value_q1616,
    int max_pixel_value_q1616
)
{
    unsigned x, y;
    unsigned scale_range = (max_pixel_value_q1616 - min_pixel_value_q1616 + 1) / 256;

    for (y = 0; y < rows; y++) {
        for (x = 0; x < cols; x++) {
            /*
             * Initial pixel values are expected to range from
             * 0x0000 to 0xFFFF in Q16.16 format.
             */
            unsigned pixel_q1616 = (unsigned)*image++;
            unsigned gray_pixel_8b = ((pixel_q1616 - min_pixel_value_q1616) / scale_range) & 0xff;

            vga_plot(x, y, gray_pixel_8b);
        }
    }
}

/* ----------------------------------------------------------------
 * Main inference routine
 */

int main()
{
    *hex = 0x3f; /* display '-' */

    volatile int* l1_b = nn;                    /* layer 1 bias */
    volatile int* l1_w = l1_b + L1_OUT;         /* layer 1 weights */
    volatile int* l2_b = l1_w + L1_IN * L1_OUT; /* layer 2 bias */
    volatile int* l2_w = l2_b + L2_OUT;         /* layer 2 weights */
    volatile int* l3_b = l2_w + L2_IN * L2_OUT; /* layer 3 bias */
    volatile int* l3_w = l3_b + L3_OUT;         /* layer 3 weights */

    int result;

    display_image(input, 28, 28, 0, 0xFFFF);

#if ACCEL_FULL_LAYER

    /*
     * Full-layer acceleration:
     * dot product, bias, and activation are handled in hardware.
     */
    wordcopy(bank0, input, L1_IN);

    apply_layer_act(L1_IN, L1_OUT, l1_b, l1_w, 1, bank0, bank1);
    apply_layer_act(L2_IN, L2_OUT, l2_b, l2_w, 1, bank1, bank0);
    apply_layer_act(L3_IN, L3_OUT, l3_b, l3_w, 0, bank0, bank1);

    result = max_index(L3_OUT, bank1);

#elif ACCEL_CACHED_DOT

    /*
     * Cached dot-product acceleration:
     * activations are reused through on-chip SRAM banks.
     */
    wordcopy(bank0, input, L1_IN);

    apply_layer_dot(L1_IN, L1_OUT, l1_b, l1_w, 1, bank0, bank1);
    apply_layer_dot(L2_IN, L2_OUT, l2_b, l2_w, 1, bank1, bank0);
    apply_layer_dot(L3_IN, L3_OUT, l3_b, l3_w, 0, bank0, bank1);

    result = max_index(L3_OUT, bank1);

#elif ACCEL_DOT_ONLY

    /*
     * Dot-product acceleration:
     * intermediate activations are stored in external SDRAM.
     */
    apply_layer_dot(L1_IN, L1_OUT, l1_b, l1_w, 1, input, l1_acts);
    apply_layer_dot(L2_IN, L2_OUT, l2_b, l2_w, 1, l1_acts, l2_acts);
    apply_layer_dot(L3_IN, L3_OUT, l3_b, l3_w, 0, l2_acts, l3_acts);

    result = max_index(L3_OUT, l3_acts);

#elif ACCEL_MEMCOPY_ONLY

    /*
     * Memory-transfer accelerator validation mode.
     */
    volatile int* dest_buffer = l1_acts;

    wordcopy(dest_buffer, input, L1_IN);

    int errors = 0;

    for (int i = 0; i < L1_IN; i++) {
        if (dest_buffer[i] != input[i]) {
            errors++;
        }
    }

    result = (errors == 0) ? 1 : 0;

#else

    /*
     * Software reference implementation with no hardware acceleration.
     */
    apply_layer_dot(L1_IN, L1_OUT, l1_b, l1_w, 1, input, l1_acts);
    apply_layer_dot(L2_IN, L2_OUT, l2_b, l2_w, 1, l1_acts, l2_acts);
    apply_layer_dot(L3_IN, L3_OUT, l3_b, l3_w, 0, l2_acts, l3_acts);

    result = max_index(L3_OUT, l3_acts);

#endif

    * hex = hex7seg(result);

    return 0;
}