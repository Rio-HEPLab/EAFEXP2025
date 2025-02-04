// do not forget to load the driver
// to compile: g++ -o read-regV1718 read-regV1718.cc -lCAENVME
// to run: ./read-regV1718 

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "/opt/CAENVMELib-v3.3.7/include/CAENVMElib.h"
#include "/opt/CAENVMELib-v3.3.7/include/CAENVMEtypes.h"
#include "/opt/CAENVMELib-v3.3.7/include/CAENVMEoslib.h"
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{

   time_t StartAcq, DetectRisingEdge, DetectFallingEdge;

   // DEFINITIONS    

   CVBoardTypes VMEBoard;
   short Link,Device;
   int32_t BHandle;

   VMEBoard = cvV1718;
   Link = 0;

   // V1718 input channels (they are inverted wrt manual)
   // Ch0: 2'b10, Ch1: 2'b01
   CVRegisters InputReg = (CVRegisters)0x8;

   // OPENING VME INTERFACE MODULE

   if ( CAENVME_Init( VMEBoard, Device, Link, &BHandle ) != cvSuccess )
   {

      printf( "Error initializing.\n" );
      return 0;

   } else {

      printf( "INFO: You have sucessfully opened the module V1718\n" );

   }

   CAENVME_SystemReset( BHandle );

   // Acquisition with V1718

   // V1718 IN register
   unsigned int retVal;
   bool regIN=false;
   bool regIN_prev=false;

   printf("\n");

   int ievt=0;
   bool RisingEdgeDetected = false;

   time( &StartAcq ); 

   while(true)
   {
      // Read V1718 IN register
      CAENVME_ReadRegister( BHandle, InputReg, &retVal );

      regIN_prev = regIN;
      regIN = ( retVal & 0x1 );

      printf( "regIN      : %d\n", regIN );

      // Detect rising and falling edges
      // Rising edge
      if ( regIN && !regIN_prev )
      {
         // Time of rising edge
	 time( &DetectRisingEdge );

	 printf( "%d: Rising edge detected.\n", DetectRisingEdge );

         RisingEdgeDetected = true;

      // Falling edge
      } else if ( !regIN && regIN_prev ) {

         // Time of falling edge
	 time( &DetectFallingEdge );

	 printf( "%d: Falling edge detected.\n", DetectFallingEdge );

         // Compute time
	 long int AcqTime = -1; 
         if ( RisingEdgeDetected ) {
            AcqTime = ( DetectFallingEdge - DetectRisingEdge );
	    printf( "Event %d. Gate duration: %d (s)\n", ievt, AcqTime );
	 } else {
            printf( "Event %d. Time from start of acquisition: %d (s).\n", ievt, ( DetectFallingEdge - StartAcq ) );
	 }

         RisingEdgeDetected = false;

	 ++ievt;

	 printf("\n");
      }

      usleep(500000); // us

   }

   CAENVME_End( BHandle );

   return 0;

}
