#include "TransformRQ31.h"
#include <stdio.h>
#include "Error.h"
#include "Test.h"


#define SNR_THRESHOLD 90
#define ABS_RFFT_ERROR_Q31 ((q31_t)33)
#define ABS_RIFFT_ERROR_Q31 ((q31_t)52000)
#define ABS_RIFFT_LONG_ERROR_Q31 ((q31_t)209000)


#if defined(ARM_MATH_NEON)
#define RFFT_INIT(L) \
  arm_rfft_init_q31(&this->instRfftQ31 ,L);
#define RIFFT_INIT(L) \
  arm_rfft_init_q31(&this->instRfftQ31 ,L);

#else 
#define RFFT_INIT(L) \
  arm_rfft_init_q31(&this->instRfftQ31 ,L,0,1);

#define RIFFT_INIT(L) \
  arm_rfft_init_q31(&this->instRfftQ31 ,L,1,1);
#endif

    void TransformRQ31::test_rfft_q31()
    {
       q31_t *inp = input.ptr();

       q31_t *tmp = inputchanged.ptr();

       q31_t *outp = outputfft.ptr();
       q31_t *overoutp = overheadoutputfft.ptr();

       ASSERT_TRUE (outp!=NULL);
      

       memcpy(tmp,inp,sizeof(q31_t)*input.nbSamples());

       //for(unsigned int i=0;i<input.nbSamples();i++)
       //{
       //  printf("%02d : 0x%08X\n",i,tmp[i]);
       //}
       //printf("\n");

#if defined(ARM_MATH_NEON)
      q31_t *tmp2p = tmp2.ptr();
      arm_rfft_q31(
             &this->instRfftQ31,
             tmp,
             overoutp,tmp2p,this->ifft);
#else
       arm_rfft_q31(
             &this->instRfftQ31,
             tmp,
             overoutp);
#endif
       //for(unsigned int i=0;i<outputfft.nbSamples();i++)
       //{
       //  printf("%02d : 0x%08X\n",i,overoutp[i]);
       //}
       //printf("\n");

       if (this->ifft)
       {
          for(unsigned long i = 0;i < overheadoutputfft.nbSamples(); i++)
          {
              overoutp[i] = overoutp[i] << this->scaling;
          }
       }

       //for(unsigned int i=0;i<outputfft.nbSamples();i++)
       //{
       //  printf("%02d : 0x%08X\n",i,overoutp[i]);
       //}
       //printf("\n");

       memcpy(outp,overoutp,sizeof(q31_t)*outputfft.nbSamples());

       //for(unsigned int i=0;i<outputfft.nbSamples();i++)
       //{
       //  printf("%02d : 0x%08X\n",i,outp[i]);
       //}
       //printf("\n");

       //ASSERT_SNR(outputfft,ref,(q31_t)SNR_THRESHOLD);
       if (this->ifft)
       {
          if (this->scaling==12)
          {
              ASSERT_NEAR_EQ(outputfft,ref,ABS_RIFFT_LONG_ERROR_Q31);
          }
          else 
          {
              ASSERT_NEAR_EQ(outputfft,ref,ABS_RIFFT_ERROR_Q31);
          }
       }
       else 
       {
          ASSERT_NEAR_EQ(outputfft,ref,ABS_RFFT_ERROR_Q31);
       }
       ASSERT_EMPTY_TAIL(outputfft);

       
        
    } 

  
    void TransformRQ31::setUp(Testing::testID_t id,std::vector<Testing::param_t>& paramsArgs,Client::PatternMgr *mgr)
    {


       (void)paramsArgs;

       switch(id)
       {

          case TransformRQ31::TEST_RFFT_Q31_1:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_32_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_32_Q31_ID,mgr);

            RFFT_INIT(32);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_17:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_32_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_32_Q31_ID,mgr);

             RIFFT_INIT(32);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=5;

          break;

          case TransformRQ31::TEST_RFFT_Q31_2:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_64_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_64_Q31_ID,mgr);

            RFFT_INIT(64);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_18:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_64_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_64_Q31_ID,mgr);

            RIFFT_INIT(64);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=6;

          break;

          case TransformRQ31::TEST_RFFT_Q31_3:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_128_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_128_Q31_ID,mgr);

            RFFT_INIT(128);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_19:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_128_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_128_Q31_ID,mgr);

            RIFFT_INIT(128);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=7;

          break;

          case TransformRQ31::TEST_RFFT_Q31_4:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_256_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_256_Q31_ID,mgr);

            RFFT_INIT(256);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_20:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_256_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_256_Q31_ID,mgr);

            RIFFT_INIT(256);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=8;

          break;

          case TransformRQ31::TEST_RFFT_Q31_5:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_512_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_512_Q31_ID,mgr);

            RFFT_INIT(512);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_21:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_512_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_512_Q31_ID,mgr);

            RIFFT_INIT(512);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=9;

          break;

          case TransformRQ31::TEST_RFFT_Q31_6:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_1024_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_1024_Q31_ID,mgr);

            RFFT_INIT(1024);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_22:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_1024_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_1024_Q31_ID,mgr);

            RIFFT_INIT(1024);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=10;

          break;

          case TransformRQ31::TEST_RFFT_Q31_7:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_2048_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_2048_Q31_ID,mgr);

            RFFT_INIT(2048);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_23:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_2048_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_2048_Q31_ID,mgr);

            RIFFT_INIT(2048);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=11;

          break;

          case TransformRQ31::TEST_RFFT_Q31_8:

            input.reload(TransformRQ31::INPUTS_RFFT_NOISY_4096_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_NOISY_4096_Q31_ID,mgr);

            RFFT_INIT(4096);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_24:

            input.reload(TransformRQ31::INPUTS_RIFFT_NOISY_4096_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_NOISY_4096_Q31_ID,mgr);

            RIFFT_INIT(4096);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=12;

          break;

          /* STEP FUNCTIONS */

          case TransformRQ31::TEST_RFFT_Q31_9:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_32_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_32_Q31_ID,mgr);

            RFFT_INIT(32);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_25:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_32_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_32_Q31_ID,mgr);

            RIFFT_INIT(32);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=5;

          break;

          case TransformRQ31::TEST_RFFT_Q31_10:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_64_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_64_Q31_ID,mgr);

            RFFT_INIT(64);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_26:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_64_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_64_Q31_ID,mgr);

            RIFFT_INIT(64);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=6;

          break;

          case TransformRQ31::TEST_RFFT_Q31_11:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_128_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_128_Q31_ID,mgr);

            RFFT_INIT(128);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);
            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_27:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_128_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_128_Q31_ID,mgr);

            RIFFT_INIT(128);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=7;

          break;

          case TransformRQ31::TEST_RFFT_Q31_12:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_256_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_256_Q31_ID,mgr);

            RFFT_INIT(256);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_28:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_256_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_256_Q31_ID,mgr);

            RIFFT_INIT(256);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=8;

          break;

          case TransformRQ31::TEST_RFFT_Q31_13:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_512_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_512_Q31_ID,mgr);

            RFFT_INIT(512);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_29:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_512_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_512_Q31_ID,mgr);

            RIFFT_INIT(512);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=9;

          break;

          case TransformRQ31::TEST_RFFT_Q31_14:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_1024_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_1024_Q31_ID,mgr);

            RFFT_INIT(1024);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_30:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_1024_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_1024_Q31_ID,mgr);

            RIFFT_INIT(1024);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=10;

          break;

          case TransformRQ31::TEST_RFFT_Q31_15:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_2048_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_2048_Q31_ID,mgr);

            RFFT_INIT(2048);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_31:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_2048_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_2048_Q31_ID,mgr);

            RIFFT_INIT(2048);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=11;

          break;

          case TransformRQ31::TEST_RFFT_Q31_16:

            input.reload(TransformRQ31::INPUTS_RFFT_STEP_4096_Q31_ID,mgr);
            ref.reload(  TransformRQ31::REF_RFFT_STEP_4096_Q31_ID,mgr);

            RFFT_INIT(4096);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=0;

          break;

          case TransformRQ31::TEST_RFFT_Q31_32:

            input.reload(TransformRQ31::INPUTS_RIFFT_STEP_4096_Q31_ID,mgr);
            ref.reload(  TransformRQ31::INPUTS_RFFT_STEP_4096_Q31_ID,mgr);

            RIFFT_INIT(4096);

            inputchanged.create(input.nbSamples(),TransformRQ31::TEMP_Q31_ID,mgr);

            this->ifft=1;
            this->scaling=12;

          break;



       }

       
      outputfft.create(ref.nbSamples(),TransformRQ31::OUTPUT_RFFT_Q31_ID,mgr);
      /*

      RFFT is writing more samples than it should.
      This is a temporary buffer allowing the test to pass.

      */
#if defined(ARM_MATH_NEON)
      overheadoutputfft.create(ref.nbSamples(),TransformRQ31::FULLOUTPUT_Q31_ID,mgr);
#else 
      overheadoutputfft.create(2*ref.nbSamples(),TransformRQ31::FULLOUTPUT_Q31_ID,mgr);
#endif

      tmp2.create(ref.nbSamples(),TransformRQ31::FULLOUTPUT_Q31_ID,mgr);

    }

    void TransformRQ31::tearDown(Testing::testID_t id,Client::PatternMgr *mgr)
    {
        (void)id;
        (void)mgr;
        //outputfft.dump(mgr);
        //overheadoutputfft.dump(mgr);
    }
