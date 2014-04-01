#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
void main(int argc, char *argv[])
{
	int cmdtime;
	if(argc>1) cmdtime=atoi(argv[1]);
	else cmdtime=-1;
	int timercount=0;
	int AC,PC[1],PC1[1],IR=0,X=0,Y=0,SP1[1],SP[1];
	int x,y,sum=0,i=0,cputomem[2],memtocpu[2],stack[2000];
	int random_number,port_value;
	PC[0]=PC1[0]=0;
	FILE *fp;
	int iflag=0;
	int stop;
	int flag[1],flag1[1];
	int temp=0,temp1=0;
	int rec[1];
	char abc[5];
	char temparray[5];
	int e;
	pipe(cputomem);
	pipe(memtocpu);
	pid_t pid;
	int readf=0;
	int tflag=0;
	pid = fork();
	SP1[0]=1000;
	switch(pid)
	{
		case -1:
		printf("Fork failed\n"); //if the process ID is -1, fork has failed
		exit(-1);
		break;
	
		case 0: //Child process- memory
		fp=fopen("Sample.txt","r");  //Reading the sample file 
		for(i=0;fscanf(fp,"%s",&abc)!=EOF;i++)
		{
			if(abc[0]=='.')	    //if a '.' is encountered, the following address is used to store the next value. i.e, jumps to that address
			{
				for(x=0;x<strlen(abc);x++)
					temparray[x]=abc[x+1];
				i=atoi(temparray);
				fscanf(fp,"%s",&abc);
			}
			stack[i]=atoi(abc);  
		}
		
		while(1)
		{
			read(cputomem[0],flag,sizeof(flag)); 
			read(cputomem[0],PC1,sizeof(PC1)); 
			if(flag[0]==0)
			{
				temp = stack[PC1[0]];
				write(memtocpu[1],&temp,sizeof(temp)); //the value at the program counter is sent to the processor
			}
			else if(flag[0]==1)
			{
				read(cputomem[0],SP,sizeof(SP));
				stack[SP[0]]=PC1[0]; //pushing to the user stack
				//printf("\n\n\nPushing into the stack at %d value %d\n\n\n",SP[0],stack[SP[0]]);
			}	
			else if(flag[0]==2)
			{
				read(cputomem[0],SP,sizeof(SP));
				temp = stack[SP[0]];   // popping from the user stack
				//printf("\n\n\nPopping from the stack at %d value %d\n\n\n",SP[0],temp);
				write(memtocpu[1],&temp,sizeof(temp));
			}
			else if(flag[0]==3)
			{
				read(cputomem[0],SP,sizeof(SP));
				temp=SP[0];                  
				read(cputomem[0],SP,sizeof(SP));
				stack[SP[0]]=temp;        
				//printf("\n\n\nPushing into the stack at %d value %d\n\n\n",SP[0],stack[SP[0]]);
				read(cputomem[0],SP,sizeof(SP));
				stack[SP[0]]=PC1[0];    //pushing to system stack
				//printf("\n\n\nPushing into the stack at %d value %d\n\n\n",SP[0],stack[SP[0]]);
			}
			else
			{
				read(cputomem[0],SP,sizeof(SP));
				temp = stack[SP[0]];
				//printf("\n\n\nPopping from the stack at %d value %d\n\n\n",SP[0],temp);
				write(memtocpu[1],&temp,sizeof(temp));
				read(cputomem[0],SP,sizeof(SP));
				temp = stack[SP[0]];   //popping from system stack
				//printf("\n\n\nPopping from the stack at %d value %d\n\n\n",SP[0],temp);
				write(memtocpu[1],&temp,sizeof(temp));
			}			
		}
		break;

		default:  //parent process-CPU
		if(PC[0]==0)
		{
			flag1[0]=0;
			write(cputomem[1],flag1,sizeof(flag1));
			write(cputomem[1],PC,sizeof(PC));  //if PC=0, write to memory
		}

		while(1)
		{
			if(timercount!=cmdtime) 
			read(memtocpu[0],&temp1,sizeof(temp1));    //read from memory only if the timer is not equal to the command line argument
			//if(tflag==0) printf("\nReading from memory %d\t",temp1);
			switch(temp1)
			{
				case 1: //Load the value into the AC
				PC[0]=PC[0]+1; //the program counter is incremented 
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC)); //PC is written to memory
				read(memtocpu[0],&IR,sizeof(IR)); //the instruction in that PC is obtained 
				AC=IR;  //this instruction is stored in the AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					temp1=29;
					iflag=1;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 2:  //Load the value at the address into the AC
				PC[0]=PC[0]+1; //program counter increments
				e=PC[0]; 
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1)); 
				write(cputomem[1],PC,sizeof(PC)); 
				read(memtocpu[0],&IR,sizeof(IR)); 
				PC[0]=IR;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR));
				AC=IR;   //The value at the following address is loaded into the AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]=e+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1)); 
				write(cputomem[1],PC,sizeof(PC));

				break;
		
				case 3: //Load the value from the address found in the address into the AC
				PC[0]=PC[0]+1;
				e=PC[0];
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1)); 
				write(cputomem[1],PC,sizeof(PC)); 
				read(memtocpu[0],&IR,sizeof(IR)); 
				PC[0]=IR;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR));
				PC[0]=IR;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR));
				AC=IR;   //The value at the following address is loaded into the AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]=e+2;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1)); 
				write(cputomem[1],PC,sizeof(PC));

				break;
				
				case 4:  //Load the value at (address+X) into the AC
				PC[0]=PC[0]+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR));
				e=PC[0];
				PC[0]=IR+X; //the PC is set to address+value of X
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC)); //the incremented PC is sent to the memory
				read(memtocpu[0],&IR,sizeof(IR)); //corresponding instruction is obtained
				AC=IR;  //the instruction is stored in the AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else 
				{	PC[0]=e;
					PC[0]++;
				}
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC)); 
				break;

				case 5: //Load the value at (address+Y) into the AC
				PC[0]=PC[0]+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR));
				e=PC[0];
				PC[0]=IR+Y;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR)); //the value at address+Y is read from memory
				AC=IR;  //this value is stored into AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else 
				{
					PC[0]=e;
					PC[0]++;
				}
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 6:  //Load from (Sp+X) into the AC
				e=X+SP1[0];
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],&e,sizeof(e));
				read(memtocpu[0],&IR,sizeof(IR)); //the value at stack pointer+X is read from memory
				AC=IR; //this value is stored into AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 7:   //Store the value in the AC into the address
				e=PC[0];
				PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR)); //the following address after this instruction is read
				PC[0]=IR;
				flag1[0]=1;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],&AC,sizeof(AC));
				write(cputomem[1],PC,sizeof(PC));
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]=e+2;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 8: //Gets a random int from 1 to 100 into the AC
				AC=rand()%100+1; //generates a random number from 1 to 100 and stores in AC
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{

					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 9:  //If port=1, writes AC as an int to the screen; If port=2, writes AC as a char to the screen

				PC[0]=PC[0]+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR));
				if(IR==1)
					printf("%d",AC); //prints AC as an integer
				else
					printf("%c",AC); //prints AC as a character
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{

					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 10:    //Add the value in X to the AC
				AC=X+AC;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 11:  //Add the value in Y to the AC
				AC=Y+AC;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 12:   //Subtract the value in X from the AC
				AC=AC-X;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{

					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 13: //Subtract the value in Y from the AC
				AC=AC-Y;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{

					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 14:   //Copy the value in the AC to X
				X=AC;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 15:   //Copy the value in X to the AC
				AC=X;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{

					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 16:   //Copy the value in the AC to Y
				Y=AC;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 17:  //Copy the value in Y to the AC
				AC=Y;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 18:   //Copy the value in AC to the SP
				SP1[0]=AC;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));		
				break;

				case 19:   //Copy the value in SP to the AC 
				AC=SP1[0];
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 20:   //Jump to the address
				PC[0]=PC[0]+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR)); //the address following this instruction is read
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]=IR; //PC is set and the control jumps to that address
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 21:   //Jump to the address only if the value in the AC is zero
				PC[0]=PC[0]+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR)); //the address following this instruction is read
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else 
				{
					if(AC==0) //if AC is zero, jump to that address
						PC[0]=IR;
					else
						PC[0]++;
				}
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 22:     //Jump to the address only if the value in the AC is not zero
				PC[0]=PC[0]+1;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				read(memtocpu[0],&IR,sizeof(IR)); //the address following this instruction is read
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else 
				{
					if(AC!=0)
						PC[0]=IR; //if AC is not zero, control jumps to that address
					else
						PC[0]++;
				}
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 23:   //Push return address onto stack, jump to the address
				PC[0]=PC[0]+2; 
				flag1[0]=1;
				SP1[0]--;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				write(cputomem[1],SP1,sizeof(SP1)); //the PC is incremented by 2 and is pushed into the stack
				flag1[0]=0;
				PC[0]--; 
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC)); //the PC is decremented again and the corresponding value is read
				read(memtocpu[0],&IR,sizeof(IR));
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]=IR; //the value read is stored into PC and control jumps to that location
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 24:   //Pop return address from the stack, jump to the address
				flag1[0]=2; //flag is set to pop form user stack
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				write(cputomem[1],SP1,sizeof(SP1));
				SP1[0]++;
				read(memtocpu[0],&IR,sizeof(IR)); //the corresponding instruction is read
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]=IR;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 25:   //Increment the value in X
				X++;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 26:   //Decrement the value in X
				X--;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 27:    //Push AC onto stack
				flag1[0]=1; //flag set to push into user stack
				SP1[0]--;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],&AC,sizeof(AC));
				write(cputomem[1],SP1,sizeof(SP1));
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 28:    //Pop from stack into AC
				flag1[0]=2; //flag set to pop from user stack
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				write(cputomem[1],SP1,sizeof(SP1));
				SP1[0]++;
				read(memtocpu[0],&IR,sizeof(IR)); //based on the flag, PC and stack pointer the instruction sent from the memory and read here
				AC=IR;
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else PC[0]++;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;

				case 29:    //Set system mode, switch stack, push SP and PC, set new SP and PC
				flag1[0]=3; //flag set to push into system stack
				write(cputomem[1],flag1,sizeof(flag1));
				PC[0]++;
				write(cputomem[1],PC,sizeof(PC));
				if(SP1[0]==1000) SP1[0]--;
				write(cputomem[1],SP1,sizeof(SP1));
				SP1[0]=1999;  //the stack pointer is set to 1999(system stack)
				write(cputomem[1],SP1,sizeof(SP1));
				SP1[0]--;
				write(cputomem[1],SP1,sizeof(SP1));
				stop=SP1[0];
				if(iflag==1) 
				{
					PC[0]=1000;
					timercount=0;
					tflag=1;
				}
				else 
				{
					PC[0]=1500;
					tflag=1;
				}
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				break;
				
				case 30:    //Restore registers, set user mode
				flag1[0]=4; //flag to pop from system stack
				SP1[0]=stop;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));
				write(cputomem[1],SP1,sizeof(SP1));
				read(memtocpu[0],&IR,sizeof(IR)); //based on the flag,PC and stack pointer the instruction is sent
				PC[0]=IR;
				tflag=0;
				iflag=0;
				SP1[0]++;
				stop=SP1[0];
				write(cputomem[1],SP1,sizeof(SP1));
				read(memtocpu[0],&IR,sizeof(IR));
				SP1[0]=IR;
				flag1[0]=0;
				write(cputomem[1],flag1,sizeof(flag1));
				write(cputomem[1],PC,sizeof(PC));				
				break;
				
				case 50:  //End execution
				if(tflag==0)timercount++;
				if(timercount==cmdtime) 
				{
					tflag=1;
					iflag=1;
					temp1=29;
					break;
				}
				else
				exit(0);  //program exits
				break;

				default: 
				printf("Error. Invalid Instruction.");

			}
			//if(tflag==0) printf("\tTimer=%d\n",timercount);		
        }
    }
}



