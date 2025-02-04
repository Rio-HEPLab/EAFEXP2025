// do not forget to load the driver
// to compile: g++ -o run-scaler run-scaler.cc -lCAENVME
// to run: ./run-scaler

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
   bool debug = false;

   // INPUT
   //
   const char* Filename = "Output_scaler.dat";
   if ( argc > 1 )
   {
      Filename = argv[1]; 
   }
   printf( "INFO: Output file name: %s\n", Filename );

   time_t StartAcq, DetectRisingEdge, DetectFallingEdge;

   // DEFINITIONS    

   CVBoardTypes VMEBoard;
   short Link,Device;
   int32_t BHandle;

   VMEBoard = cvV1718;
   Link = 0;

   CVAddressModifier Am          = cvA24_U_DATA;
   CVAddressModifier AmArray[16] = { Am, Am, Am, Am, Am, Am, Am, Am, Am, Am, Am, Am, Am, Am, Am, Am };

   CVDataWidth Dw32          = cvD32;
   CVDataWidth Dw32Array[16] = { Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32, Dw32 };
   CVErrorCodes ECs[16];

   // SETTINGS

   uint32_t Counts[16];
   uint32_t AddressBase = 0xB00000;

   uint32_t RegReadResetCh01	= (AddressBase+0x40);
   uint32_t RegReadResetCh02	= (AddressBase+0x44);
   uint32_t RegReadResetCh03	= (AddressBase+0x48);
   uint32_t RegReadResetCh04	= (AddressBase+0x4C);
   uint32_t RegReadResetCh05	= (AddressBase+0x50);
   uint32_t RegReadResetCh06	= (AddressBase+0x54);
   uint32_t RegReadResetCh07	= (AddressBase+0x58);
   uint32_t RegReadResetCh08	= (AddressBase+0x5C);
   uint32_t RegReadResetCh09	= (AddressBase+0x60);
   uint32_t RegReadResetCh10	= (AddressBase+0x64);
   uint32_t RegReadResetCh11	= (AddressBase+0x68);
   uint32_t RegReadResetCh12	= (AddressBase+0x6C);
   uint32_t RegReadResetCh13	= (AddressBase+0x70);
   uint32_t RegReadResetCh14	= (AddressBase+0x74);
   uint32_t RegReadResetCh15	= (AddressBase+0x78);
   uint32_t RegReadResetCh16	= (AddressBase+0x7C);

   uint32_t AddressArray[16] = {
      RegReadResetCh01, RegReadResetCh02, RegReadResetCh03, RegReadResetCh04,
      RegReadResetCh05, RegReadResetCh06, RegReadResetCh07, RegReadResetCh08,
      RegReadResetCh09, RegReadResetCh10, RegReadResetCh11, RegReadResetCh12,
      RegReadResetCh13, RegReadResetCh14, RegReadResetCh15, RegReadResetCh16
   };

   // V1718 input channels 
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

   // Acquisition with V1718 and SCALER 

   // V1718 IN register
   unsigned int retVal;
   bool regIN=false;
   bool regIN_prev=false;

   printf("\n");
   FILE *f = fopen( Filename, "w" );
   printf( "HEADER: ievnt channel counts aquisition_time rate\n" );
   fprintf( f, "ievnt channel counts aquisition_time rate\n" );
   fclose(f);

   int ievt=0;
   bool RisingEdgeDetected = false;

   time( &StartAcq ); 

   while(true)
   {
      // Read V1718 IN register
      CAENVME_ReadRegister( BHandle, InputReg, &retVal );

      regIN_prev = regIN;
      regIN = ( retVal & 0x1 );

      if ( debug ) {
         printf( "regIN prev.: %d\n", regIN_prev );
         printf( "regIN      : %d\n", regIN );
      }

      // Detect rising and falling edges
      // Rising edge
      if ( regIN && !regIN_prev )
      {
         // Time of rising edge
	 time( &DetectRisingEdge );

	 if ( debug )
	    printf( "%d: Rising edge detected.\n", DetectRisingEdge );
	 else
	    printf( "Acquisition started.\n");

         RisingEdgeDetected = true;

      // Falling edge
      } else if ( !regIN && regIN_prev ) {

         // Time of falling edge
	 time( &DetectFallingEdge );

	 if ( debug )
	    printf( "%d: Falling edge detected.\n", DetectFallingEdge );

         // Read SCALER counts and reset registers
	 CAENVME_MultiRead( BHandle, AddressArray, Counts, 16, AmArray, Dw32Array, ECs );

	 if ( debug ) {
	    for ( unsigned int ch = 0; ch < 16; ++ch ) {
               printf( "Counts channel %d: %d\n", ch, Counts[ ch ] );
	    }
	 }

         // Compute time
	 long int AcqTime = -1; 
         if ( RisingEdgeDetected ) { AcqTime = ( DetectFallingEdge - DetectRisingEdge ); }
	 else { printf( "Time from start of acquisition: %d (s).\n", ( DetectFallingEdge - StartAcq ) ); }

	 // Print counts per channel
	 for ( unsigned int ch = 0; ch < 16; ++ch ) {

	    if ( Counts[ch] > 0 ) {

	       float Rate = -1.;
               if ( RisingEdgeDetected ) { Rate = float(Counts[ch]) / AcqTime; }

	       printf( "%d %d %zu %d %f\n", ievt, (ch+1), Counts[ch], AcqTime, Rate );

	       FILE *f = fopen(Filename, "a");
	       fprintf( f, "%d %d %zu %d %f\n", ievt, (ch+1), Counts[ch], AcqTime, Rate );
	       fclose(f);
	    }
	 }           

         RisingEdgeDetected = false;

	 ++ievt;

	 printf("\n");
      }

      usleep(1000);

   }

   CAENVME_End( BHandle );

   return 0;

}
