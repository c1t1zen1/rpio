#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <netinet/tcp.h>
#include <unistd.h>     
#include <sys/time.h>

#include <math.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>


#define ADC_SPI_CHANNEL 1
#define ADC_SPI_SPEED 1000000
#define ADC_NUM_CHANNELS 6
#define SMOOTH 0  // 0 = no smoothing; 1 = 2x; 2 = 4x; etc
#define RESOLUTION 1023 // 1023 if using MCP3008; 4095 if using MCP3208

void die(char *errorMessage)
{
  perror(errorMessage);
  exit(1);
}

// interrupt things

static volatile uint8_t TR1_flag = 0;
static volatile uint8_t TR2_flag = 0;
static volatile uint8_t TR3_flag = 0;
static volatile uint8_t B1_flag  = 0;
static volatile uint8_t B2_flag  = 0;


void Interrupt_TR1 (void) { TR1_flag = 1; }
void Interrupt_TR2 (void) { TR2_flag = 1; }
void Interrupt_TR3 (void) { TR3_flag = 1; }
void Interrupt_B1  (void) { B1_flag  = 1; }
void Interrupt_B2  (void) { B2_flag  = 1; }

// ADC 

uint16_t readADC(int _channel){ // 12 bit

	uint8_t spi_data[3];
	uint8_t input_mode = 1; // single ended = 1, differential = 0
	uint16_t result;

	//if(_channel > 7 | _channel < 0){
	//	return -1;
	//}


	spi_data[0] = 0x04; // start flag
	spi_data[0] |= (input_mode<<1); // shift input_mode
	spi_data[0] |= (_channel>>2) & 0x01; // add msb of channel in our first command byte

	spi_data[1] = _channel<<6;
	spi_data[2] = 0x00;

	wiringPiSPIDataRW(ADC_SPI_CHANNEL, spi_data, 3);

	result = (spi_data[1] & 0x0f)<<8 | spi_data[2];
	return result;
}

uint16_t readADC_10bit(int _channel){

	uint8_t spi_data[3];
	uint16_t result;

	//if(_channel > 7 | _channel < 0){
	//	return -1;
	//}


	spi_data[0] = 0x01; // start flag
	spi_data[1] = (_channel + 8) << 4;
	spi_data[2] = 0x00;

	wiringPiSPIDataRW(ADC_SPI_CHANNEL, spi_data, 3);

	result = (spi_data[1] & 3)<<8 | spi_data[2];
	return result;
}


int main(int argc, char *argv[]){

	int sock;                        /* Socket descriptor */
    	struct sockaddr_in echoServAddr; /* Echo server address */
    	unsigned short echoServPort;     /* Echo server port */
    	char *servIP;                    /* Server IP address (dotted quad) */
    	int ms_rate;                     /* ms_rate (time between each send) */
     	int value = 1;
 

    	if (argc != 4)    /*  correct number of arguments ? */
    	{
      		 fprintf(stderr, "Usage: %s <Server IP> <Server Port> <ms_rate>\n", argv[0]);
       		 exit(1);
    	}

   	servIP = argv[1];              /* server IP address */
   	echoServPort = atoi(argv[2]);  /* server port */
    	ms_rate = atoi(argv[3]);       /* time between each send (in milliseconds) */

    	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        	die("socket() failed");

    	
    	memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    	echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    	echoServAddr.sin_port        = htons(echoServPort); /* Server port */

    	/* connection to the echo server */
    	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        die("connect() failed");

    	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof(int)) < 0)
      	die("TCP_NODELAY failed");

    	delay(100);


	wiringPiSetupSys();
	wiringPiSPISetup(ADC_SPI_CHANNEL, ADC_SPI_SPEED);

	wiringPiISR (24, INT_EDGE_FALLING, &Interrupt_TR1) ;
  	wiringPiISR (22, INT_EDGE_FALLING, &Interrupt_TR2) ;
  	wiringPiISR (23, INT_EDGE_FALLING, &Interrupt_TR3) ;
  	wiringPiISR (14, INT_EDGE_FALLING, &Interrupt_B1) ;
  	wiringPiISR ( 3, INT_EDGE_FALLING, &Interrupt_B2) ;

	int _repeatADC = pow(2, SMOOTH);

	for(;;){

		// digital inputs: interrupt?

		if (TR1_flag) { 
			char FUDI[5] = {'6', '\t', '1', ';', '\0'};
			TR1_flag = 0;
			//printf("trigger 1");
			send(sock, FUDI, strlen(FUDI), 0);
		}
		if (TR2_flag) { 
			char FUDI[5] = {'7', '\t', '1', ';', '\0'};
			TR2_flag = 0;	
			//printf("trigger 2");
			send(sock, FUDI, strlen(FUDI), 0);
		}
		if (TR3_flag) { 
			char FUDI[5] = {'8', '\t', '1', ';', '\0'};
			TR3_flag = 0;
			//printf("trigger 3");
			send(sock, FUDI, strlen(FUDI), 0);
		}
		if ( B1_flag) { 
			char FUDI[5] = {'9', '\t', '1', ';', '\0'};
			B1_flag =  0;
			send(sock, FUDI, strlen(FUDI), 0);
		}
		if ( B2_flag) { 
			char FUDI[6] = {'1', '0', '\t', '1', ';', '\0'}; 
			B2_flag =  0;
			send(sock, FUDI, strlen(FUDI), 0);
		}

                /// ADC: 
	     
		for(int i = 0; i < ADC_NUM_CHANNELS; i++){

			int16_t val = 0;
			uint8_t msgLength;
			char *ADC_FUDI;
			//val =  rand() % 2000;
			//val = RESOLUTION - readADC_10bit(i+2);

			for (int j = 0; j < _repeatADC; j++) {
				val += readADC_10bit(i+2);
		       		}
			val = RESOLUTION - (val>>SMOOTH);
			
			if (val > 999)        { msgLength = 8; }
			else if (val > 99)    { msgLength = 7; }
			else if (val > 9)     { msgLength = 6; }
			else 		      { msgLength = 5; }
			/* format FUDI msg: id, whitespace, val, semicolon, /0 */
			ADC_FUDI = (char *) malloc(msgLength); 
			snprintf(ADC_FUDI+2, msgLength, "%d", val);
			ADC_FUDI[0] = (char)(((int)'0')+(5-i));
			ADC_FUDI[1] = ' ';
			ADC_FUDI[msgLength-2] = ';';
			//strcat(ADC_FUDI, ";");
			send(sock, ADC_FUDI, strlen(ADC_FUDI), 0);
			delay(ms_rate);
						
		}

	
       		
	}
}
