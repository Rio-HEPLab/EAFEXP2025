// g++ -o test-scaler test-scaler.cc -lCAENVME 
// to run: ./test-scaler
//

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
   // INPUT
   //
   const char* Filename = "Output_scaler.dat";
   if ( argc > 1 )
   {
      Filename = argv[1]; 
   }
   printf( "INFO: Output file name: %s\n", Filename );

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

   uint32_t RegReadResetCh01 = (AddressBase+0x40);
   uint32_t RegReadResetCh02 = (AddressBase+0x44);
   uint32_t RegReadResetCh03 = (AddressBase+0x48);
   uint32_t RegReadResetCh04 = (AddressBase+0x4C);
   uint32_t RegReadResetCh05 = (AddressBase+0x50);
   uint32_t RegReadResetCh06 = (AddressBase+0x54);
   uint32_t RegReadResetCh07 = (AddressBase+0x58);
   uint32_t RegReadResetCh08 = (AddressBase+0x5C);
   uint32_t RegReadResetCh09 = (AddressBase+0x60);
   uint32_t RegReadResetCh10 = (AddressBase+0x64);
   uint32_t RegReadResetCh11 = (AddressBase+0x68);
   uint32_t RegReadResetCh12 = (AddressBase+0x6C);
   uint32_t RegReadResetCh13 = (AddressBase+0x70);
   uint32_t RegReadResetCh14 = (AddressBase+0x74);
   uint32_t RegReadResetCh15 = (AddressBase+0x78);
   uint32_t RegReadResetCh16 = (AddressBase+0x7C);

   uint32_t AddressArray[16] = {
      RegReadResetCh01, RegReadResetCh02, RegReadResetCh03, RegReadResetCh04,
      RegReadResetCh05, RegReadResetCh06, RegReadResetCh07, RegReadResetCh08,
      RegReadResetCh09, RegReadResetCh10, RegReadResetCh11, RegReadResetCh12,
      RegReadResetCh13, RegReadResetCh14, RegReadResetCh15, RegReadResetCh16
   };

   // OPENING VME INTERFACE MODULE

   if ( CAENVME_Init( VMEBoard, Device, Link, &BHandle ) != cvSuccess )
   {

      printf( "Error initializing.\n" );
      return 0;


   } else {

      printf( "INFO: You have sucessfully opened the module V1718\n" );

   }

   // READ SCALER

   unsigned int retVal;
   int ievt=1;

   printf("\n");
   FILE *f = fopen( Filename, "w" );
   printf( "HEADER: ievnt channel counts\n" );
   fprintf( f, "ievnt channel counts\n" );

   CAENVME_MultiRead( BHandle, AddressArray, Counts, 16, AmArray, Dw32Array, ECs );

   for ( int ch = 0; ch < 16; ++ch )
   {            
      if ( Counts[ch] > 0 )
      {
	 printf( "%d %d %zu\n", ievt, (ch+1), Counts[ch] );
	 fprintf( f, "%d %d %zu\n", ievt,(ch+1), Counts[ch] );
      }
   }          
   fclose(f);

   ievt++;

   printf("\n");

   CAENVME_End( BHandle );

   return 0;

}
