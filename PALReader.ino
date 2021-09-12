/*
 * PALReader - Dump out ROMs and PALs using an Arduino ATmega2560
 *
 * www.github.com/hharte/PALReader
 *
 * (c) 2021, Howard M. Harte
 *
 */

#define N82S181       /* Signetics 82S181, Motorola MCM7681PC */
#define INTEL_HEX     /* Output in Intel Hex format */
#define BAUD_RATE 115200

#ifdef N82S181
#define ROM_PART_NUMBER "82S181"
#define ROM_SIZE  1024

#define ADDR7     2
#define ADDR6     3
#define ADDR5     4
#define ADDR4     5
#define ADDR3     6
#define ADDR2     7
#define ADDR1     8
#define ADDR0     9
#define ADDR8     10
#define ADDR9     11
#define CE1       12
#define CE2       13
#endif

#define BIT0      (1 << 0)
#define BIT1      (1 << 1)
#define BIT2      (1 << 2)
#define BIT3      (1 << 3)
#define BIT4      (1 << 4)
#define BIT5      (1 << 5)
#define BIT6      (1 << 6)
#define BIT7      (1 << 7)
#define BIT8      (1 << 8)
#define BIT9      (1 << 9)


void setup() {
  /* Initialize serial port */
  Serial.begin(BAUD_RATE);

  /* Configure Port F for input (D7:0) */
  DDRF = 0x00;
  PORTF = 0x00;

  /* Configure Address and chip enables. */
  pinMode (ADDR0, OUTPUT);
  pinMode (ADDR1, OUTPUT);
  pinMode (ADDR2, OUTPUT);
  pinMode (ADDR3, OUTPUT);
  pinMode (ADDR4, OUTPUT);
  pinMode (ADDR5, OUTPUT);
  pinMode (ADDR6, OUTPUT);
  pinMode (ADDR7, OUTPUT);  
  pinMode (ADDR8, OUTPUT);
  pinMode (ADDR9, OUTPUT);  
  pinMode (CE1,   OUTPUT);
  pinMode (CE2,   OUTPUT);
  /* CE3# and CE4# hardwired to GND. */
  
  while (!Serial) {
    ; /* wait for USB serial port to connect */
  }

  Serial.print("\n\nPALReader (c) 2021 - Howard M. Harte\n");
  Serial.print("https://github.com/hharte/PALReader\n\n");
  Serial.print("Dumping " ROM_PART_NUMBER "...\n");
}

unsigned int AddressWord = 0;
unsigned int DataByte = 0xA5;
char outstr[81];
unsigned int cksum;

#define RECORD_LEN  16

void loop() {

  if (AddressWord != ROM_SIZE) {
    /* Set current ROM address A9:0 pins */
    digitalWrite(ADDR9, (AddressWord & BIT9) && 1);
    digitalWrite(ADDR8, (AddressWord & BIT8) && 1);
    digitalWrite(ADDR7, AddressWord & BIT7);
    digitalWrite(ADDR6, AddressWord & BIT6);
    digitalWrite(ADDR5, AddressWord & BIT5);
    digitalWrite(ADDR4, AddressWord & BIT4);
    digitalWrite(ADDR3, AddressWord & BIT3);
    digitalWrite(ADDR2, AddressWord & BIT2);
    digitalWrite(ADDR1, AddressWord & BIT1);
    digitalWrite(ADDR0, AddressWord & BIT0);
    
    /* Assert chip enables */
    digitalWrite(CE1,   1);
    digitalWrite(CE2,   1);
  }

  /* Read data byte from Port F */
  DataByte = PINF; 

  /* Deassert chip enables */
  digitalWrite(CE1,   0);
  digitalWrite(CE2,   0);

  if (AddressWord % RECORD_LEN == 0) {
    if (AddressWord > 0) {
      cksum = ~cksum + 1;
      cksum &= 0xFF; 
#ifdef INTEL_HEX
      snprintf(outstr, sizeof(outstr), "%02X", cksum);
      Serial.print(outstr);
#endif /* INTEL_HEX */

      if (AddressWord == ROM_SIZE) {
#ifdef INTEL_HEX
        /* End of file record. */
        snprintf(outstr, sizeof(outstr), "\n:00000001FF");
        Serial.print(outstr);
#endif /* INTEL_HEX */
        snprintf(outstr, sizeof(outstr), "\n\nDump complete: %d bytes\n\n", AddressWord);
        Serial.print(outstr);
        Serial.print("Press reset button to dump another " ROM_PART_NUMBER ".\n");
        
        /* Loop forever until reset. */
        while (true) {
          continue;
        }
      }
    }

    cksum = RECORD_LEN + ((AddressWord >> 8) & 0xFF) + (AddressWord & 0xFF) + DataByte;
#ifdef INTEL_HEX
    snprintf(outstr, sizeof(outstr), "\n:%02X%04X00%02X", RECORD_LEN, AddressWord, DataByte);
#else
    snprintf(outstr, sizeof(outstr), "\n%04X: %02X", AddressWord, DataByte);
#endif /* INTEL_HEX */
  } else {
    cksum += DataByte;
#ifdef INTEL_HEX
    snprintf(outstr, sizeof(outstr), "%02X", DataByte);
#else
    snprintf(outstr, sizeof(outstr), " %02X", DataByte);
#endif /* INTEL_HEX */    
  }
  Serial.print(outstr);

  /* Advance to the next address */
  AddressWord++;
}
