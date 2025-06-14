/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_mat_mult_fast_q15.c
 * Description:  Q15 matrix multiplication (fast variant)
 *
 * $Date:        23 April 2021
 * $Revision:    V1.9.0
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dsp/matrix_functions.h"

/**
  @ingroup groupMatrix
 */

/**
  @addtogroup MatrixMult
  @{
 */

/**
  @brief         Q15 matrix multiplication (fast variant).
  @param[in]     pSrcA      points to the first input matrix structure
  @param[in]     pSrcB      points to the second input matrix structure
  @param[out]    pDst       points to output matrix structure
  @param[in]     pState     points to the array for storing intermediate results
  @return        execution status
                   - \ref ARM_MATH_SUCCESS       : Operation successful
                   - \ref ARM_MATH_SIZE_MISMATCH : Matrix size check failed

  @par           Scaling and Overflow Behavior
                   The difference between the function \ref arm_mat_mult_q15() and this fast variant is that
                   the fast variant use a 32-bit rather than a 64-bit accumulator.
                   The result of each 1.15 x 1.15 multiplication is truncated to
                   2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30
                   format. 
  @par
                   The fast version has the same overflow behavior as the standard version but provides
                   less precision since it discards the low 16 bits of each multiplication result.
                   In order to avoid overflows completely the input signals must be scaled down.
                   Scale down one of the input matrices by log2(numColsA) bits to avoid overflows,
                   as a total of numColsA additions are computed internally for each output element.
  @remark
                   Refer to \ref arm_mat_mult_q15() for a slower implementation of this function
                   which uses 64-bit accumulation to provide higher precision.
 */
#if defined(ARM_MATH_NEON) 
#define LANE 8
#define DTYPE q15_t
#define HEADERTYPE DTYPE
#define VEC int16x8_t
#define VECACC int32x4x2_t

#define CLEAR_ACC(tmp) tmp.val[0] = vdupq_n_s32(0); \
 tmp.val[1] = vdupq_n_s32(0);

#define TMPMAC \
 int32x4_t tmp;

#define TMPLD \
  VEC tmpld;

#define TMPST       \
  int16x4x2_t htmp;

#if defined(ARM_DSP_TESTING)
int cov_mat_mul_fast_q15[20]={0};
#define LOGKERNEL(A,B) cov_mat_mul_fast_q15[B]=1;
#else
#define LOGKERNEL(A,B) 
#endif

#define SCALARACC int32_t 
#define SCALAR_LOAD_AND_WIDEN(DST,PTR) DST = (SCALARACC)(*(PTR))
#define SCALAR_STORE_AND_NARROW(PTR,VAL) *(PTR) = (q15_t) __SSAT((VAL) >> 15, 16)
#define SCALAR_MAC_N(ACC,VEC,SCALAR) ACC += (SCALARACC)(VEC) * (SCALARACC)(SCALAR)

#define VLOAD(DST,PTR) DST = vld1q_s16((PTR))
#define VSTORE(PTR,VAL) vst1q_s16((PTR),(VAL))

#define VLOAD_ACC(DST,PTR)         \
  DST.val[0] = vld1q_s32((PTR)+4*0); \
  DST.val[1] = vld1q_s32((PTR)+4*1);
  
#define VSTORE_ACC(PTR,VAL)        \
  vst1q_s32((PTR)+4*0,(VAL).val[0]); \
  vst1q_s32((PTR)+4*1,(VAL).val[1]);

#define VLOAD_AND_WIDEN(DST,PTR)           \
    tmpld = vld1q_s16((PTR));                    \
    DST.val[0] = vmovl_s16(vget_low_s16(tmpld)); \
    DST.val[1] = vmovl_s16(vget_high_s16(tmpld));

#define VSTORE_AND_NARROW(PTR,VAL) \
    htmp.val[0] = vqshrn_n_s32(VAL.val[0],15);    \
    htmp.val[1] = vqshrn_n_s32(VAL.val[1],15);    \
    vst1q_s16(PTR,vcombine_s16(htmp.val[0],htmp.val[1]));

#define VMAC_N(ACC,VEC,SCALAR)                             \
   tmp = vmull_s16(vget_low_s16(VEC),vdup_n_s16(SCALAR));  \
   ACC.val[0] = vqaddq_s32(ACC.val[0],tmp);                \
   tmp = vmull_s16(vget_high_s16(VEC),vdup_n_s16(SCALAR)); \
   ACC.val[1] =vqaddq_s32(ACC.val[1],tmp);

#define MATTYPE arm_matrix_instance_q15
#define EXT(A) A##_fast_q15
#define HAS_TEMP_BUFFER
#define USE_TMP_REGISTER

#define FUNCNAME arm_mat_mult_fast_q15

#include "_arm_mat_mult_neon.c"

#else
ARM_DSP_ATTRIBUTE arm_status arm_mat_mult_fast_q15(
        const arm_matrix_instance_q15 * pSrcA,
        const arm_matrix_instance_q15 * pSrcB,
              arm_matrix_instance_q15 * pDst,
              q15_t                   * pState)
{
        q31_t sum;                                     /* Accumulator */
        q15_t *pSrcBT = pState;                        /* Input data matrix pointer for transpose */
        q15_t *pInA = pSrcA->pData;                    /* Input data matrix pointer A of Q15 type */
        q15_t *pInB = pSrcB->pData;                    /* Input data matrix pointer B of Q15 type */
        q15_t *px;                                     /* Temporary output data matrix pointer */
        uint16_t numRowsA = pSrcA->numRows;            /* Number of rows of input matrix A */
        uint16_t numColsB = pSrcB->numCols;            /* Number of columns of input matrix B */
        uint16_t numColsA = pSrcA->numCols;            /* Number of columns of input matrix A */
        uint16_t numRowsB = pSrcB->numRows;            /* Number of rows of input matrix B */
        uint32_t col, i = 0U, row = numRowsB, colCnt;  /* Loop counters */
        arm_status status;                             /* Status of matrix multiplication */

#if defined (ARM_MATH_DSP)
        q31_t in;                                      /* Temporary variable to hold the input value */
        q31_t inA1, inB1, inA2, inB2;
        q31_t sum2, sum3, sum4;
        q15_t *pInA2, *pInB2, *px2;
        uint32_t j = 0;
#else
        q15_t in;                                      /* Temporary variable to hold the input value */
        q15_t inA1, inB1, inA2, inB2;
#endif /* #if defined (ARM_MATH_DSP) */

#ifdef ARM_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if ((pSrcA->numCols != pSrcB->numRows) ||
      (pSrcA->numRows != pDst->numRows)  ||
      (pSrcB->numCols != pDst->numCols)    )
  {
    /* Set status as ARM_MATH_SIZE_MISMATCH */
    status = ARM_MATH_SIZE_MISMATCH;
  }
  else

#endif /* #ifdef ARM_MATH_MATRIX_CHECK */

  {
    /* Matrix transpose */
    do
    {
      /* The pointer px is set to starting address of column being processed */
      px = pSrcBT + i;

      /* Apply loop unrolling and exchange columns with row elements */
      col = numColsB >> 2U;

      /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
       ** a second loop below computes the remaining 1 to 3 samples. */
      while (col > 0U)
      {

#if defined (ARM_MATH_DSP)

        /* Read two elements from row */
        in = read_q15x2_ia (&pInB);

        /* Unpack and store one element in destination */
#ifndef ARM_MATH_BIG_ENDIAN
        *px = (q15_t) in;
#else
        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);
#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

        /* Update pointer px to point to next row of transposed matrix */
        px += numRowsB;

        /* Unpack and store second element in destination */
#ifndef ARM_MATH_BIG_ENDIAN
        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);
#else
        *px = (q15_t) in;
#endif /* #ifndef ARM_MATH_BIG_ENDIAN */

        /* Update pointer px to point to next row of transposed matrix */
        px += numRowsB;

        in = read_q15x2_ia (&pInB);
#ifndef ARM_MATH_BIG_ENDIAN
        *px = (q15_t) in;
#else
        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);
#endif /* #ifndef ARM_MATH_BIG_ENDIAN */
        px += numRowsB;

#ifndef ARM_MATH_BIG_ENDIAN
        *px = (q15_t) ((in & (q31_t) 0xffff0000) >> 16);
#else
        *px = (q15_t) in;
#endif /* #ifndef ARM_MATH_BIG_ENDIAN */
        px += numRowsB;

#else /* #if defined (ARM_MATH_DSP) */

        /* Read one element from row */
        in = *pInB++;

        /* Store one element in destination */
        *px = in;

        /* Update pointer px to point to next row of transposed matrix */
        px += numRowsB;

        in = *pInB++;
        *px = in;
        px += numRowsB;

        in = *pInB++;
        *px = in;
        px += numRowsB;

        in = *pInB++;
        *px = in;
        px += numRowsB;

#endif /* #if defined (ARM_MATH_DSP) */

        /* Decrement column loop counter */
        col--;
      }

      /* If the columns of pSrcB is not a multiple of 4, compute any remaining output samples here.
       ** No loop unrolling is used. */
      col = numColsB % 0x4U;

      while (col > 0U)
      {
        /* Read and store input element in destination */
        *px = *pInB++;

        /* Update pointer px to point to next row of transposed matrix */
        px += numRowsB;

        /* Decrement column loop counter */
        col--;
      }

      i++;

      /* Decrement row loop counter */
      row--;

    } while (row > 0U);

    /* Reset variables for usage in following multiplication process */
    row = numRowsA;
    i = 0U;
    px = pDst->pData;

#if defined (ARM_MATH_DSP)
    /* Process two rows from matrix A at a time and output two rows at a time */
    row = row >> 1U;
    px2 = px + numColsB;
#endif

    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    while (row > 0U)
    {
      /* For every row wise process, column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, pIn2 pointer is set to starting address of transposed pSrcB data */
      pInB = pSrcBT;

#if defined (ARM_MATH_DSP)
      /* Process two (transposed) columns from matrix B at a time */
      col = col >> 1U;
      j = 0;
#endif

      /* column loop */
      while (col > 0U)
      {
        /* Set variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Initiate pointer pInA to point to starting address of column being processed */
        pInA = pSrcA->pData + i;

#if defined (ARM_MATH_DSP)
        sum2 = 0;
        sum3 = 0;
        sum4 = 0;
        pInB  = pSrcBT + j;
        pInA2 = pInA + numColsA;
        pInB2 = pInB + numRowsB;

        /* Read in two elements at once - allows dual MAC instruction */
        colCnt = numColsA >> 1U;
#else
        colCnt = numColsA >> 2U;
#endif

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1) * b(1,1) + a(1,2) * b(2,1) + .... + a(m,p) * b(p,n) */

#if defined (ARM_MATH_DSP)
          /* read real and imag values from pSrcA and pSrcB buffer */
          inA1 = read_q15x2_ia (&pInA);
          inB1 = read_q15x2_ia (&pInB);

          inA2 = read_q15x2_ia (&pInA2);
          inB2 = read_q15x2_ia (&pInB2);

          /* Multiply and Accumulates */
          sum  = __SMLAD(inA1, inB1, sum);
          sum2 = __SMLAD(inA1, inB2, sum2);
          sum3 = __SMLAD(inA2, inB1, sum3);
          sum4 = __SMLAD(inA2, inB2, sum4);
#else
          /* read real and imag values from pSrcA and pSrcB buffer */
          inA1 = *pInA++;
          inB1 = *pInB++;
          /* Multiply and Accumulates */
          sum += inA1 * inB1;

          inA2 = *pInA++;
          inB2 = *pInB++;
          sum += inA2 * inB2;

          inA1 = *pInA++;
          inB1 = *pInB++;
          sum += inA1 * inB1;

          inA2 = *pInA++;
          inB2 = *pInB++;
          sum += inA2 * inB2;
#endif /* #if defined (ARM_MATH_DSP) */

          /* Decrement loop counter */
          colCnt--;
        }

        /* process odd column samples */
#if defined (ARM_MATH_DSP)
        if (numColsA & 1U) {
          inA1 = *pInA++;
          inB1 = *pInB++;
          inA2 = *pInA2++;
          inB2 = *pInB2++;
          sum  += inA1 * inB1;
          sum2 += inA1 * inB2;
          sum3 += inA2 * inB1;
          sum4 += inA2 * inB2;
        }
#else
        colCnt = numColsA % 0x4U;

        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1) * b(1,1) + a(1,2) * b(2,1) + .... + a(m,p) * b(p,n) */
          sum += (q31_t) *pInA++ * *pInB++;

          /* Decrement loop counter */
          colCnt--;
        }
#endif /* #if defined (ARM_MATH_DSP) */

        /* Saturate and store result in destination buffer */
        *px++  = (q15_t) (sum >> 15);

#if defined (ARM_MATH_DSP)
        *px++  = (q15_t) (sum2 >> 15);
        *px2++ = (q15_t) (sum3 >> 15);
        *px2++ = (q15_t) (sum4 >> 15);
        j += numRowsB * 2;
#endif

        /* Decrement column loop counter */
        col--;

      }

      i = i + numColsA;

#if defined (ARM_MATH_DSP)
      i = i + numColsA;
      px = px2 + (numColsB & 1U);
      px2 = px + numColsB;
#endif

      /* Decrement row loop counter */
      row--;

    }

    /* Compute any remaining odd row/column below */

#if defined (ARM_MATH_DSP)

    /* Compute remaining output column */
    if (numColsB & 1U) {

      /* Avoid redundant computation of last element */
      row = numRowsA & (~0x1);

      /* Point to remaining unfilled column in output matrix */
      px = pDst->pData + numColsB-1;
      pInA = pSrcA->pData;

      /* row loop */
      while (row > 0)
      {

        /* point to last column in matrix B */
        pInB  = pSrcBT + numRowsB * (numColsB-1);

        /* Set variable sum, that acts as accumulator, to zero */
        sum  = 0;

        /* Compute 4 columns at once */
        colCnt = numColsA >> 2U;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          inA1 = read_q15x2_ia (&pInA);
          inA2 = read_q15x2_ia (&pInA);
          inB1 = read_q15x2_ia (&pInB);
          inB2 = read_q15x2_ia (&pInB);

          sum  = __SMLAD(inA1, inB1, sum);
          sum  = __SMLAD(inA2, inB2, sum);

          /* Decrement loop counter */
          colCnt--;
        }

        colCnt = numColsA & 3U;
        while (colCnt > 0U) {
          sum += (q31_t) (*pInA++) * (*pInB++);
          colCnt--;
        }

        /* Store result in destination buffer */
        *px = (q15_t) (sum  >> 15);
        px += numColsB;

        /* Decrement row loop counter */
        row--;
      }
    }

    /* Compute remaining output row */
    if (numRowsA & 1U) {

      /* point to last row in output matrix */
      px = pDst->pData + (numColsB) * (numRowsA-1);

      pInB  = pSrcBT;
      col = numColsB;
      i = 0U;

      /* col loop */
      while (col > 0)
      {
        /* point to last row in matrix A */
        pInA = pSrcA->pData + (numRowsA-1) * numColsA;

        /* Set variable sum, that acts as accumulator, to zero */
        sum  = 0;

        /* Compute 4 columns at once */
        colCnt = numColsA >> 2U;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          inA1 = read_q15x2_ia (&pInA);
          inA2 = read_q15x2_ia (&pInA);
          inB1 = read_q15x2_ia (&pInB);
          inB2 = read_q15x2_ia (&pInB);

          sum  = __SMLAD(inA1, inB1, sum);
          sum  = __SMLAD(inA2, inB2, sum);

          /* Decrement loop counter */
          colCnt--;
        }

        colCnt = numColsA % 4U;
        while (colCnt > 0U) {
          sum += (q31_t) (*pInA++) * (*pInB++);

          colCnt--;
        }

        /* Store result in destination buffer */
        *px++ = (q15_t) (sum  >> 15);

        /* Decrement column loop counter */
        col--;
      }
    }

#endif /* #if defined (ARM_MATH_DSP) */

    /* Set status as ARM_MATH_SUCCESS */
    status = ARM_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}
#endif
/**
  @} end of MatrixMult group
 */
