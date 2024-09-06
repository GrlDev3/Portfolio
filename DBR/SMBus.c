#include <CPU_init.h>
#include <SMBus.h>
#include <Util.h>
#include <SysTick.h>
#include <UART.h>

xdata volatile U16 CH;
xdata volatile U8 SMB_DATA_IN[SMB_IN_SIZE];
xdata volatile U8 SMB_DATA_OUT;
xdata volatile U8 *pSMB_DATA_OUT = &SMB_DATA_OUT;
bit SB_OK;
bit SMB_RW;                            
bit proba=0;


void Free_I2C (void)
{
   U8 i;
   while(!SDA)
   {
      XBR2 = 0x40;                     // Enable Crossbar
      SCL = 0;                         // Drive the clock low
      for(i = 0; i < 254; i++);        // Hold the clock low
      SCL = 1;                         // Release the clock
      while(!SCL);                     // Wait for open-drain
                                       // clock output to rise
      for(i = 0; i < 10; i++);         // Hold the clock high
      XBR2 = 0x00;                     // Disable Crossbar
   }
}

void SMBus_ISR (void) interrupt 7
{
   bit FAIL = 0;                       // Used by the ISR to flag failed
                                       // transfers 
	
   static U8 sent_byte_counter;
   static U8 rec_byte_counter;

      // Normal operation
      switch (SMB0STA)           // Status vector
      {
         // Master Transmitter/Receiver: START condition transmitted.
         case SMB_MTSTA:
            SMB0DAT = ADC;        	   // Load address of the target slave
                                       // R/W bit
            SMB0DAT |= SMB_RW;         // Load R/W bit
            STA = 0;                   // Manually clear START bit
            rec_byte_counter = 0;      // Reset the counter
            sent_byte_counter = 1;     // Reset the counter
            break;

         // Master Transmitter: Data byte transmitted
         case SMB_MTDB:					//0x18 Slave Address + W transmitted. ACK received.
 			if (AA)                   // Slave ACK?
 			{	   
			   if (SMB_RW == WRITE)    // If this transfer is a WRITE,
               {
                  if (sent_byte_counter <= 1)
                  {
                     // send data byte
                     SMB0DAT = SMB_DATA_OUT;
                     sent_byte_counter++;
			  
				  }
                  else
                  {
                     
					 STO = 1;          // Set STO to terminate transfer
					 
					 
                  }
               }
               else {}                 // If this transfer is a READ,
                                       // proceed with transfer without
                                       // writing to SMB0DAT (switch
                                       // to receive mode)


            }
            else                       // If slave NACK,
            {
               STO = 1;                // Send STOP condition, followed
			   
               
            }
            break;

         // Master Receiver: byte received
         case SMB_MRAA:				//0x40 Slave Address + R transmitted. ACK received.

			if (rec_byte_counter <= NUM_BYTES_RD)
            {
                                                          // byte
               AA = 1;                // Send ACK to indicate byte received
            }
            else
            {
               AA = 0;                // Send NACK to indicate last byte

               STO = 1;                // Send STOP to terminate transfer
			   //SB_BUSY=0;
            }
            break;
         
		 case SMB_MTLD:				//0x28 Data byte transmitted. ACK received
				proba = 1;
               	AA = 0;
			   	STO = 1;
			                   // Send STOP to terminate transfer
			
		 break;
		 
		 case SMB_MRLD:				//0x58 Data byte received. NACK transmitted.
               AA = 0;
			   STO = 1;                // Send STOP to terminate transfer
			   SB_OK=1;
			   SB_BUSY=0;
		 break;




         case SMB_MRDA:					//0x50  Data byte received. ACK transmitted.
            if (rec_byte_counter < NUM_BYTES_RD)
            {
               SMB_DATA_IN[rec_byte_counter] = SMB0DAT; // Store received
               
			   AA = 1;                // Send ACK to indicate byte received
               rec_byte_counter++;     // Increment the byte counter
            }
            else
			{
				//SB_BUSY=0;
				AA = 0;                // Send NACK to indicate last byte
            }                       // of this transfer
            break;

		case 0x48:
			STO=1;
			STA=1;
			break;

         default:
            FAIL = 1;                  // Indicate failed transfer
            STO = 1;                // Send STOP condition, followed
			                           // and handle at end of ISR
            break;

      } // end switch

   SI = 0;                             // Clear interrupt flag
}

//-----------------------------------------------------------------
// Format:   register , command , lenght
//-----------------------------------------------------------------

	 


void Timer3_ISR (void) interrupt 14
{
  
   SMB0CN &= ~0x40;                    // Disable SMBus
   SMB0CN |= 0x40;                     // Re-enable SMBus
   TMR3CN &= ~0x80;           //0x08         // Clear Timer3 interrupt-pending flag
   STA = 0;
	
}


