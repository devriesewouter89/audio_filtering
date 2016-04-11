
 // local includes
#include <dsp.h>

// arm cmsis library includes
#define ARM_MATH_CM4

#include "stm32f4xx.h"
#include <arm_math.h>
//#include "uart.h"
// arm c library includes
#include <stdbool.h>

// the user button switch
extern volatile int user_mode;
int old_user_mode;

#define NUM_FIR_TAPS 56
#define BLOCKSIZE    512

// allocate the buffer signals and the filter coefficients
arm_fir_instance_q15 FIR;
q15_t outSignal[BLOCKSIZE];

q15_t fir_coeffs_bp[NUM_FIR_TAPS] = {   -8,  -35,  -56,  -40,   -3,  -18, -121, -223, 
																			-185,  -29,   18, -213, -536, -544, -139,  186, -138, 
																			-940,-1263, -478,  563,  557, -983,-3051,-1867, 3035, 
																			8993,11666, 8993, 3035,-1867,-3051, -983,  557,  563, 
																			-478,-1263, -940, -138,  186, -139, -544, -536, -213,   
																			  18,  -29, -185, -223, -121,  -18,   -3,  -40,  -56, -35, 
																			  -8,    0,};  // band pass at 150Hz to 3KHz with 40dB at 1.5Khz for SR=16KHz

q15_t fir_bp_475_575[81] = { 324,   49, -497,  -35,  700,  102, -944, -112, 1046, 
																			 176,-1057, -188,  846,  228, -493, -213,  -34,  205, 
																		   584, -144,-1096,  84,  1388,   14,-1387,  -98,  980,  
																			 191, -440, -942,  288, 2258, -276,-3597,  237, 4733,
																			-153,-5508,   57,    5,  774,   57,-5508, -153, 4733,  
																			 237,-3597, -276, 2258,  288, -942, -248, -192,  191,  
									 										 980,  -98,-1387,   14, 1388,   84,-1096, -144,  584, 
																			 205,  -34, -213, -493,  228,  846, -188,-1057,  176, 
																			1046, -112, -944,  102,  700,  -35, -497,   49,  324 };

q15_t fir_bp_815_825[200] = {  21, -181,   47,  -40,   -3,   51,  -91, 109,   -93, 
																			  44,   28, -102,  153, -163,  121,  -34, -76,   175,
																			-230,  218, -135,    0,  150, -268,  314, -265,  128, 
																			  63, -250,  376, -394,  291,  -89,  -156, 372, -487,
																			 460, -288,   16,  279, -505,  590,  -498, 247,   93, 
																			-421,  637, -669,  497, -166, -230,  572, -752,  710,
																			-451,   47,  385, -713,  834, -707,  362, 100,  -542, 
																			 829, -872,  653, -234, -261,  684,  -906, 859, -554, 
																			  81,  418, -793,  933, -793,  418,    81, -554, 859, 
																			-906,  684, -261, -234,  653, -872,   829, -542, 100,
																			 362, -707,  834, -713,  385,   47,  -451, 710, -752, 
																			 572, -230, -166,  497, -669,  637,  -421, 93,   247,
																			-498,  590, -505,  279,   16, -288,   460, -487, 372, 
																			-156,  -89,  291, -394,  376, -250,    63, 128, -265,
																			 314, -268,  150,    0, -135,  218,  -230, 175,  -76,
																			 -34,  121, -163,  153, -102,   28,    44, -93,  109,
																		   -91,   51,   -3,  -40,   47, -181,    21 };
//q15_t fir_coeffs_bp[NUM_FIR_TAPS] = {};
//q15_t fir_coeffs_bp[NUM_FIR_TAPS] = {};
//q15_t fir_coeffs_bp[NUM_FIR_TAPS] = {};
	
q15_t fir_state[NUM_FIR_TAPS + BLOCKSIZE];
bool firstStart = false;

// the core dsp function
void dsp(int16_t* buffer, int length)
{
	// only enable the filter if the user button is pressed
	if (user_mode & 1)
	{
	  // we initiate the filter only if needed to prevent clitches at the beginning of new buffers
		if (firstStart == false || old_user_mode != user_mode)
		{
			initFilter();
			old_user_mode = user_mode;
			firstStart = true;
		}
		
  	// process with FIR
	  arm_fir_fast_q15(&FIR, buffer, outSignal, BLOCKSIZE);
		
  	// copy the result
	  arm_copy_q15(outSignal, buffer, length);
  }
	if (user_mode & 2)
	{
	  // we initiate the filter only if needed to prevent clitches at the beginning of new buffers
		if (firstStart == false || old_user_mode != user_mode)
		{
			initFilter();
			old_user_mode = user_mode;
			firstStart = true;
		}
		
  	// process with FIR
	  arm_fir_fast_q15(&FIR, buffer, outSignal, BLOCKSIZE);
		
  	// copy the result
	  arm_copy_q15(outSignal, buffer, length);
  }
}

// we initialize and switch the filter here
void initFilter()
{
  // apply the band pass filter
  if (user_mode & 1)
    arm_fir_init_q15(&FIR, NUM_FIR_TAPS, fir_bp_475_575, fir_state, BLOCKSIZE);
		
  // or applay the high pass filter depending on the user button switch mode
  if (user_mode & 2)
    arm_fir_init_q15(&FIR, NUM_FIR_TAPS, fir_coeffs_bp, fir_state, BLOCKSIZE);
	// apply both
	//else {
	//	arm_fir_init_q15(&FIR, NUM_FIR_TAPS, fir_coeffs_hp, fir_state, BLOCKSIZE);
	//	arm_fir_init_q15(&FIR, NUM_FIR_TAPS, fir_coeffs_lp, fir_state, BLOCKSIZE);
		
	//}
}

//-------------------------------------------------------------------------------------------------------

// below is a test environment for a noice cancelation with adaptive filters
 // local includes
/*#include <dsp.h>

// arm cmsis library includes
#define ARM_MATH_CM4
#include "stm32f4xx.h"
#include <arm_math.h>

// arm c library includes
#include <stdbool.h>

// the user button switch
extern volatile int user_mode;

#define NUM_FIR_TAPS 128
#define BLOCKSIZE    512
#define MU           1

// allocate the buffer signals and the filter coefficients on the heap
q15_t fir_coeffs_bp[NUM_FIR_TAPS] = {   -8,  -35,  -56,  -40,   -3,  -18, -121, -223, 
																			-185,  -29,   18, -213, -536, -544, -139,  186, -138, 
																			-940,-1263, -478,  563,  557, -983,-3051,-1867, 3035, 
																			8993,11666, 8993, 3035,-1867,-3051, -983,  557,  563, 
																			-478,-1263, -940, -138,  186, -139, -544, -536, -213,   
																			  18,  -29, -185, -223, -121,  -18,   -3,  -40, -56, -35, 
																			  -8,    0,};  // band pass at 150Hz to 3KHz with 40dB at 1.5Khz for SR=16KHz

arm_lms_instance_q15 LMS; 
q15_t outSignal[BLOCKSIZE];
q15_t refSignal[BLOCKSIZE];
q15_t errSignal[BLOCKSIZE];
q15_t fir_coeffs[NUM_FIR_TAPS];
q15_t state[NUM_FIR_TAPS + BLOCKSIZE];
q15_t fir_state[NUM_FIR_TAPS + BLOCKSIZE];
bool firstStart = false;
arm_fir_instance_q15 FIR;
// the core dsp function
void dsp(int16_t* buffer, int length)
{
	// this block is only processed one time while startup
  if (firstStart == false)
  {
    // set the filter to an ideal filter
		arm_fill_q15(1, fir_coeffs, NUM_FIR_TAPS);fir_coeffs[0] = 32767;
		// set the reference random signal
		arm_fill_q15(0, refSignal, BLOCKSIZE);
		// initialize the adaptive filter 
		arm_lms_init_q15(&LMS, NUM_FIR_TAPS, fir_coeffs, state, MU, BLOCKSIZE, 0);
		// initialize the FIR filter 
    firstStart = true;
  }
	
	// store the reference silent signa in the train mode and switch back to normal mode
  if (user_mode & 1)
	{
		// store the silent noise
    arm_copy_q15(buffer, refSignal, BLOCKSIZE);
		//USART_puts("need food "); // just send a message to indicate that it works
    arm_fir_init_q15(&FIR, NUM_FIR_TAPS, fir_coeffs_bp, fir_state, BLOCKSIZE);
		// set the train mode back
    user_mode ++;
	}
	else 
    // aplly the adaptive filter
		arm_lms_q15(&LMS, buffer, refSignal, outSignal, errSignal, BLOCKSIZE);

  // copy the result
  arm_copy_q15(errSignal, buffer, BLOCKSIZE);
}
 */
//--------------------------------------------------------------------------------------------------------- 

/* --> Below the implementation of an iir filter function without the arm cmsis library
// local includes
#include "dsp.h"
 
// the user button switch
extern volatile int user_mode;
 
// our core dsp function
void dsp(int16_t* buffer, int length)
{
  // initialize some values
  static float previous;
  int i;
     
  // if switched on, apply the filter
  if (user_mode & 1)
  {     
    // perform an simple first order high pass with 12dB/octave
    for (i=0; i<length; i++)
    {
      buffer[i] = (int16_t)( (float)buffer[i] -(float)previous * 0.75f );
      previous = buffer[i];
    }
  }
}
*/
