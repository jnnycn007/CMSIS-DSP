#include "Test.h"
#include "Pattern.h"

#include "dsp/transform_functions.h"

class TransformNeonCQ15:public Client::Suite
    {
        public:
            TransformNeonCQ15(Testing::testID_t id);
            virtual void setUp(Testing::testID_t,std::vector<Testing::param_t>& paramsArgs,Client::PatternMgr *mgr);
            virtual void tearDown(Testing::testID_t,Client::PatternMgr *mgr);
        private:
            #include "TransformNeonCQ15_decl.h"
            
            Client::Pattern<q15_t> input;
            Client::LocalPattern<q15_t> outputfft;
            Client::LocalPattern<q15_t> bufferfft;

            Client::LocalPattern<q15_t> inputfft;
            Client::LocalPattern<q15_t> tmp2;

            Client::RefPattern<q15_t> ref;

            arm_cfft_instance_q15 *varInstCfftQ15Ptr;

            uint8_t ifft;

            /*  

            ifft pattern is using the output of the fft and the input of the fft.
            Since output of the fft is scaled, the input is not recovered without an additional scaling.


            */
            int scaling;

            arm_status status;
            
    };
