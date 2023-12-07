#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include <string.h>          // strcmp
#include <stdint.h>          // C99 integer types -- uint32_t
#include <stdbool.h>         // bool
#include <fcntl.h>           // open
#include <sys/mman.h>        // mmap
#include <unistd.h>          // close

#define WAVE_BASE_OFFSET       0X00020000 
#define AXI4_LITE_BASE         0x43C00000 //base address of axi

#define OFS_MODE       		0
#define OFS_RUN        		1
#define OFS_FREQ_A     		2
#define OFS_FREQ_B     		3
#define OFS_OFFSET     		4
#define OFS_AMPLITUDE      5
#define OFS_DUTY_CYCLE		6
#define OFS_CYCLES			7

#define SPAN_IN_BYTES 32

#define GPIO_IRQ 80

uint32_t *base = NULL;

bool gpioOpen()
{
    // Open /dev/mem
    int file = open("/dev/mem", O_RDWR | O_SYNC);
	 printf("");
    bool bOK = (file >= 0);
    if (bOK)
    {
        // Create a map from the physical memory location of
        // /dev/mem at an offset to LW avalon interface
        // with an aperature of SPAN_IN_BYTES bytes
        // to any location in the virtual 32-bit memory space of the process
        base = mmap(NULL, SPAN_IN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED,
                    file, AXI4_LITE_BASE + WAVE_BASE_OFFSET);
        bOK = (base != MAP_FAILED);

        // Close /dev/mem
        close(file);
    }
    return bOK;
}

void dc(int channel, int16_t ofs, int8_t mode) //dc command 
{
	if(channel == 0)// if channel a, clear the lower 16 bits and set
	{
		*(base+OFS_OFFSET) &= 0xFFFF0000;
		*(base+OFS_OFFSET) |= ofs;
		
		*(base+OFS_MODE) &= ~0x7; // clears lower 3 bits
		*(base+OFS_MODE) |= mode;
	}
	else if(channel == 1) // if channel b,  clear the upper 16 bits and set upper 16 by bitshifting
	{
		uint32_t ofs_buffer = ofs;
		ofs_buffer <<= 16;
		*(base+OFS_OFFSET) &= 0x0000FFFF; //clears upper 3 bits
		*(base+OFS_OFFSET) |= ofs_buffer;	
		
		*(base+OFS_MODE) &= 0xFFFFFFC7; //clears bits 5:3
		*(base+OFS_MODE) |= mode;
	}		
}

void run(int channel) //run either chanlle a, b or both by a+b
{
	switch(channel)
	{
		case 0:
			*(base+OFS_RUN) |= 1;
			break;
		case 1:
			*(base+OFS_RUN) |= 2;
			break;
		case 2:
			*(base+OFS_RUN) |= 3;
			break;
	}
}

void stop(int channel)// stop either channel a, b or both by a+b
{
	uint32_t mask;
	switch(channel)
	{
		case 0:
			mask = 1;
			*(base+OFS_RUN) &= ~mask;
			break;
		case 1:
			mask = 2;
			*(base+OFS_RUN) &= ~mask;
			break;
		case 2:
			mask = 3;
			*(base+OFS_RUN) &= ~mask;
			break;
	}
}

void waves(int channel, int frequency, int16_t amplitude, int16_t ofs, int8_t mode)
{
	if(channel == 0)// if channel a, clear the lower 16 bits and set
	{
		*(base+OFS_OFFSET) &= 0xFFFF0000;
		*(base+OFS_OFFSET) |= ofs;
		
		*(base+OFS_AMPLITUDE) &= 0xFFFF0000;
		*(base+OFS_AMPLITUDE) |= amplitude;
		
		*(base+OFS_FREQ_A) &= 0x00000000;
		*(base+OFS_FREQ_A) |= frequency;
		
		*(base+OFS_MODE) &= ~0x7; //1000 clears lower 3 bits
		*(base+OFS_MODE) |= mode;
	}
	else if(channel == 1) // if channel b,  clear the upper 16 bits and set upper 16 by bitshifting
	{
		uint32_t ofs_buffer = ofs << 16;
		uint32_t amplitude_buffer = amplitude << 16;
		
		*(base+OFS_OFFSET) &= 0x0000FFFF;
		*(base+OFS_OFFSET) |= ofs_buffer;	
		
		*(base+OFS_AMPLITUDE) &= 0x0000FFFF;
		*(base+OFS_AMPLITUDE) |= amplitude_buffer;
		
		*(base+OFS_FREQ_B) &= 0x00000000;
		*(base+OFS_FREQ_B) |= frequency;
		
		*(base+OFS_MODE) &= 0xFFFFFFC7; //clears bits 5:3
		*(base+OFS_MODE) |= (mode << 3);
	}		
}

void cycles(int channel, int16_t cycl) // set the numnber of cycles from 1 to 65535 or continuous
{
		printf("in cycle fn, cycl = %d\n", cycl);
		int32_t cycl_buffer = cycl;
		cycl_buffer <<= 16;
		switch(channel){
		case 0:
			*(base+OFS_CYCLES) &= 0xFFFF0000;
			*(base+OFS_CYCLES) |= cycl;
			break;
		case 1:
			*(base+OFS_CYCLES) &= 0x0000FFFF;
			*(base+OFS_CYCLES) |= cycl_buffer;
			break;
	}
}

void square(int channel, int frequency, int16_t amplitude, int16_t ofs, int8_t mode, int8_t duty_cycl)
{
	if(channel == 0)// if channel a, clear the lower 16 bits and set
	{
		*(base+OFS_OFFSET) &= 0xFFFF0000;
		*(base+OFS_OFFSET) |= ofs;
		
		*(base+OFS_AMPLITUDE) &= 0xFFFF0000;
		*(base+OFS_AMPLITUDE) |= amplitude;
		
		*(base+OFS_FREQ_A) &= 0x00000000;
		*(base+OFS_FREQ_A) |= frequency;
		
		*(base+OFS_MODE) &= ~0x7; //1000 clears lower 3 bits
		*(base+OFS_MODE) |= mode;
		
		*(base+OFS_DUTY_CYCLE) &= 0xFFFF0000;
		*(base+OFS_DUTY_CYCLE) |= duty_cycl;
	}
	else if(channel == 1) // if channel b,  clear the upper 16 bits and set upper 16 by bitshifting
	{
		uint32_t ofs_buffer = ofs;
		uint32_t amplitude_buffer = amplitude;
		uint32_t duty_cycl_buffer = duty_cycl;
		
		ofs_buffer <<= 16;
		amplitude_buffer <<= 16;
		duty_cycl_buffer <<= 16;
		
		*(base+OFS_OFFSET) &= 0x0000FFFF;
		*(base+OFS_OFFSET) |= ofs_buffer;	
		
		*(base+OFS_AMPLITUDE) &= 0x0000FFFF;
		*(base+OFS_AMPLITUDE) |= amplitude_buffer;
		
		*(base+OFS_FREQ_B) &= 0x00000000;
		*(base+OFS_FREQ_B) |= frequency;
		
		*(base+OFS_MODE) &= 0xFFFFFFC7; //clears bits 5:3
		*(base+OFS_MODE) |= (mode << 3);
		
		*(base+OFS_DUTY_CYCLE) &= 0x0000FFFF;
		*(base+OFS_DUTY_CYCLE) |= duty_cycl_buffer;
	}		
}

void printRegs()
{
	printf("\n\nChannel A:\n");
	printf("Mode: %d\n", *(base+OFS_MODE) & 0x00000007);
	printf("Run: %d\n", *(base+OFS_RUN) & 0x00000001);
	printf("Freq_a: %d\n", *(base+OFS_FREQ_A));
	printf("Offset: %d\n", *(base+OFS_OFFSET) & 0x0000FFFF);
	printf("Amplitude: %d\n", *(base+OFS_AMPLITUDE) & 0x0000FFFF);
	printf("Dtycyc: %d\n", *(base+OFS_DUTY_CYCLE) & 0x0000FFFF);
	printf("Cycles: %d\n\n", *(base+OFS_CYCLES) & 0x0000FFFF);
	
	printf("Channel B:\n");
	printf("\nMode: %d\n", *(base+OFS_MODE)>>3);
	printf("Run: %d\n", *(base+OFS_RUN)>>1);
	printf("Freq_b: %d\n", *(base+OFS_FREQ_B));
	printf("Offset: %d\n", *(base+OFS_OFFSET)>>16);
	printf("Amplitude: %d\n", *(base+OFS_AMPLITUDE)>>16);
	printf("Dtycyc: %d\n", *(base+OFS_DUTY_CYCLE)>>16);
	printf("Cycles: %d\n\n", *(base+OFS_CYCLES)>>16);
}

void resetRegs()
{
	*(base+OFS_MODE) = 0;
	*(base+OFS_RUN) = 0;
	*(base+OFS_FREQ_A) = 0;
	*(base+OFS_FREQ_B) = 0;
	*(base+OFS_OFFSET) = 0;
	*(base+OFS_AMPLITUDE) = 0;
	*(base+OFS_DUTY_CYCLE) = 0;
	*(base+OFS_CYCLES) = 0;
}

int main(int argc, char* argv[])
{
	int16_t ofs; //used to store offset of register
	int8_t mode;
	int16_t cycl; //used to store the number of cycles
	int channel; //holds channel a or b
	int frequency; //holds frequency
	int16_t amplitude; //holds amplitude
	int8_t duty_cycl;
	int valid;

	if (argc == 2)
    {
		gpioOpen();
		if((strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
		{
			printf("  Commands:\n");
			printf("  wave dc OUT OFS        				configure dc wave where out is a or b\n");
			printf("  wave cycles OUT N		  				cycle for waveform OUT limited to N cycles\n");
			printf("  wave cycle CONTINUOUS  				the waveform will be continous\n");
			printf("  wave sine OUT FREQ AMP [OFS]  		configure sine wave with frequency in Hz and amplitude with an average of OFS. default OFS is 0\n");
			printf("  wave sawtooth OUT FREQ AMP [OFS] 		configure sawtooth wave for a or b with frequency in Hz and amplitude with an average of OFS. default OFS is 0\n\n");
			printf("  wave triangle OUT FREQ AMP [OFS]		configure triangle wave for a or b with frequency in Hz and amplitude with an average of OFS. default OFS is 0\n\n");
			printf("  wave square OUT FREQ AMP [OFS, [DC]]	configure square wave for a or b with frequency in Hz and amplitude with an average of OFS and a duty cycle. default OFS is 0. default duty cycle is 50%\n");
			printf("  wave run a b a+b						both waveforms on a and b should start with the last configured values. If not configured before, degault is 0v\n");
			printf("  wave stop a b a+b						both waveforms should stop and 0v should be output from both outputs\n");
			printf("  \n");
		}
		else if(strcmp(argv[1], "registers") == 0)
		{
			printRegs();
		}
		else if(strcmp(argv[1], "reset") == 0)
		{
			resetRegs();
		}
    }
	else if (argc == 3)
	{
		gpioOpen();
		if(strcmp(argv[1], "run") == 0)  // ./wave run a, b or a+b
		{
			if(strcmp(argv[2], "a") == 0)
			{
				run(0);
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				run(1);
			}
			else if(strcmp(argv[2], "a+b") == 0)
			{
				run(2);
			}
			else
				printf("Invalid channel. Specify \"a\" or \"b\" or \"a+b\".\n");
		}
		else if(strcmp(argv[1], "stop") == 0)  // ./wave stop a, b or a+b
		{
			if(strcmp(argv[2], "a") == 0)
			{
				stop(0);
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				stop(1);
			}
			else if(strcmp(argv[2], "a+b") == 0)
			{
				stop(2);
			}
			else
				printf("Invalid channel. Specify \"a\" or \"b\" or \"a+b\".\n");
		}
	}
	else if (argc == 4)
	{
		gpioOpen();
		valid = 0;
		if(strcmp(argv[1], "dc") == 0) // ./wave dc a or b [-25000, 25000]
		{
			if(strcmp(argv[2], "a") == 0)
			{
				channel = 0;
				valid = 1;
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				channel = 1;
				valid = 1;
			}
			else 
				printf("Invalid channel. Specify \"a\" or \"b\".\n");
		
			if(valid)
			{
				valid = 0;
				if(argv[3][0] == '-') //this checks the first character of the string for a negative sign
				{
					if(atoi(argv[3]+1) <= 25000)
					{
						valid = 1;
						ofs = atoi(argv[3]+1) * -1;
					}
					else
						printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV)\n");
				}
				else if(atoi(argv[3]) <= 25000)
				{
					valid = 1;
					ofs = atoi(argv[3]);
				}
				else
					printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV\n");
			
				if(valid)
				{
					mode = 0;
					dc(channel, ofs, mode);
				}
			}
		}
		else if(strcmp(argv[1], "cycles") == 0) //wave cycles a,b N
		{
			if(strcmp(argv[2], "a") == 0)
			{
				channel = 0;
				valid = 1;
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				channel = 1;
				valid = 1;
			}
			else 
				printf("Invalid channel. Specify \"a\" or \"b\".\n");
			
			if(valid)
			{
				valid = 0;
				if(strcmp(argv[3], "continuous")==0)
				{
					valid =1;
					cycl = 0;
				}
				else if(atoi(argv[3]) <= 65535 && atoi(argv[3]) >= 1)
				{
					valid = 1;
					cycl = atoi(argv[3]);
				}
				else
					printf("Invalid amount of cycles. Enter a number from 1 to 65535 \n");
				
				if(valid)
				{
					cycles(channel, cycl);
				}
			}
		}
	}
	else if(argc == 6)
	{
		gpioOpen();
		valid = 0;
		if(strcmp(argv[1], "sine") == 0) 
		{
			if(strcmp(argv[2], "a") == 0)
			{
				valid = 1;
				channel = 0;
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				valid = 1;
				channel = 1;
			}
			else 
				printf("Invalid channel. Specify \"a\" or \"b\".\n");
			
			if(valid)
			{
				valid = 0;
				if(atoi(argv[3]) <= 25000 && atoi(argv[3]) >= 1)
				{
					valid = 1;
					frequency = atoi(argv[3]);
				}
				else
					printf("Out of range. Enter a frequency from 1 to 25000 (units of 100uV\n");
			
				if(valid)
				{
					valid = 0;
					if(atoi(argv[4]) <= 50000 && atoi(argv[4]) >= 0)
					{
						valid = 1;
						amplitude = atoi(argv[4]);
					}
					else
						printf("Out of range. Enter a amplitude from 0 to 25000 (units of 100uV)\n");
					
					if(valid)
					{
						valid = 0;
						if(argv[5][0] == '-') //this checks the first character of the string for a negative sign
						{
							if(atoi(argv[5]+1) <= 25000)
							{
								valid = 1;
								ofs = atoi(argv[5]+1) * -1;
							}
							else
								printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV)\n");
						}
						else if(atoi(argv[5]) <= 25000)
						{
							valid = 1;
							ofs = atoi(argv[5]);
						}
						else
							printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV\n");
					}
					
					if(valid)
					{
						mode = 1;
						waves(channel, frequency, amplitude, ofs, mode);
					}
				}
			}
		}
		else if(strcmp(argv[1], "sawtooth") == 0) 
		{
			if(strcmp(argv[2], "a") == 0)
			{
				valid = 1;
				channel = 0;
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				valid = 1;
				channel = 1;
			}
			else 
				printf("Invalid channel. Specify \"a\" or \"b\".\n");
			
			if(valid)
			{
				valid = 0;
				if(atoi(argv[3]) <= 25000 && atoi(argv[3]) >= 1)
				{
					valid = 1;
					frequency = atoi(argv[3]);
				}
				else
					printf("Out of range. Enter a frequency from 1 to 25000 (units of 100uV\n");
			
				if(valid)
				{
					valid = 0;
					if(atoi(argv[4]) <= 50000 && atoi(argv[4]) >= 0)
					{
						valid = 1;
						amplitude = atoi(argv[4]);
					}
					else
						printf("Out of range. Enter a amplitude from 0 to 25000 (units of 100uV)\n");
					
					if(valid)
					{
						valid = 0;
						if(argv[5][0] == '-') //this checks the first character of the string for a negative sign
						{
							if(atoi(argv[5]+1) <= 25000)
							{
								valid = 1;
								ofs = atoi(argv[5]+1) * -1;
							}
							else
								printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV)\n");
						}
						else if(atoi(argv[5]) <= 25000)
						{
							valid = 1;
							ofs = atoi(argv[5]);
						}
						else
							printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV\n");
					}
					
					if(valid)
					{
						mode = 2;
						waves(channel, frequency, amplitude, ofs, mode);
					}
				}
			}
		}
		else if(strcmp(argv[1], "triangle") == 0) 
		{
			if(strcmp(argv[2], "a") == 0)
			{
				valid = 1;
				channel = 0;
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				valid = 1;
				channel = 1;
			}
			else 
				printf("Invalid channel. Specify \"a\" or \"b\".\n");
			
			if(valid)
			{
				valid = 0;
				if(atoi(argv[3]) <= 25000 && atoi(argv[3]) >= 1)
				{
					valid = 1;
					frequency = atoi(argv[3]);
				}
				else
					printf("Out of range. Enter a frequency from 1 to 25000 (units of 100uV\n");
			
				if(valid)
				{
					valid = 0;
					if(atoi(argv[4]) <= 50000 && atoi(argv[4]) >= 0)
					{
						valid = 1;
						amplitude = atoi(argv[4]);
					}
					else
						printf("Out of range. Enter a amplitude from 0 to 25000 (units of 100uV)\n");
					
					if(valid)
					{
						valid = 0;
						if(argv[5][0] == '-') //this checks the first character of the string for a negative sign
						{
							if(atoi(argv[5]+1) <= 25000)
							{
								valid = 1;
								ofs = atoi(argv[5]+1) * -1;
							}
							else
								printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV)\n");
						}
						else if(atoi(argv[5]) <= 25000)
						{
							valid = 1;
							ofs = atoi(argv[5]);
						}
						else
							printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV\n");
					}
					
					if(valid)
					{
						mode = 3;
						waves(channel, frequency, amplitude, ofs, mode);
					}
				}
			}
		}
	}
	else if(argc == 7)
	{
		gpioOpen();
		valid = 0;
		if(strcmp(argv[1], "square") == 0)
		{
			if(strcmp(argv[2], "a") == 0)
			{
				valid = 1;
				channel = 0;
			}
			else if(strcmp(argv[2], "b") == 0)
			{
				valid = 1;
				channel = 1;
			}
			else 
				printf("Invalid channel. Specify \"a\" or \"b\".\n");
			
			if(valid)
			{
				valid = 0;
				if(atoi(argv[3]) <= 25000 && atoi(argv[3]) >= 1)
				{
					valid = 1;
					frequency = atoi(argv[3]);
				}
				else
					printf("Out of range. Enter a frequency from 1 to 25000 (units of 100uV\n");
			
				if(valid)
				{
					valid = 0;
					if(atoi(argv[4]) <= 50000 && atoi(argv[4]) >= 0)
					{
						valid = 1;
						amplitude = atoi(argv[4]);
					}
					else
						printf("Out of range. Enter a amplitude from 0 to 25000 (units of 100uV)\n");
					
					if(valid)
					{
						valid = 0;
						if(argv[5][0] == '-') //this checks the first character of the string for a negative sign
						{
							if(atoi(argv[5]+1) <= 25000)
							{
								valid = 1;
								ofs = atoi(argv[5]+1) * -1;
							}
							else
								printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV)\n");
						}
						else if(atoi(argv[5]) <= 25000)
						{
							valid = 1;
							ofs = atoi(argv[5]);
						}
						else
							printf("Out of range. Enter a offset from -25000 to 25000 (units of 100uV\n");
						
						if(valid)
						{
							valid = 0;
							if(atoi(argv[6]) <= 100 && atoi(argv[6]) >= 0)
							{
								valid = 1;
								duty_cycl = atoi(argv[6]);
							}
							else
								printf("Out of range. Enter a Duty Cycle from -0 to 100 by 0.01\% (units of 100uV\n");
						}
						
					}				
					if(valid)
					{
						mode = 4;
						square(channel, frequency, amplitude, ofs, mode, duty_cycl);
					}
				}
			}
		}
	}
   else
      printf("  command not understood\n");
	
   return EXIT_SUCCESS;
}