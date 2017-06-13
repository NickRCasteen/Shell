#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#define TKN_BUFFER 1128

char* CurrDurr = NULL; //Global Current directory

int Check_In_Redir(char** ToCheck, int CMDSIZE2)
{
        int i;
        int Redir=0;
        for(i=0 ; i<CMDSIZE2 ; i++)
        {

                if(strcmp(ToCheck[i] , "<") == 0)
                {
                        Redir=i; //Save our Redirection index
                }

        }
        return Redir;

}

int Check_Out_Redir(char** AlsoCheck, int CMDSIZE3)
{

        int i;
        int Redir=0;
        for(i=0 ; i<CMDSIZE3 ; i++)
        {

                if(strcmp(AlsoCheck[i] , ">") == 0)
                {
                        Redir=i; //Save our Redirection index
                }

        }
        return Redir;


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>!!!!! REDIRECTION IS DONE !!!!!<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void Capture_Output(char** CMD_LIST, int CMD_SIZE, int InputChange, int OutputChange)
{

	int i;
	size_t GetDBuff = 0; //Basically for getting input from a file
	int IsBuiltIn = 0;
	FILE *Reader; //What we use for input redirect
	char* Inpoot = NULL; //Used any time input is mediary. From a file, mostly.
	char* Outpoot = malloc(sizeof(char*)*TKN_BUFFER); //The output to send back...
	char** NBICMD_LIST = NULL; //Used to editing none built in Input.
	int status;
	pid_t ProcID;
	//FIRST: CHECK FOR BUILT IN. IF YES, PROCEED TO THE BUILT IN SECTION, READING OFF OUR INPUT CHANGE.
	if(strcmp(CMD_LIST[0], "intro") == 0 || strcmp(CMD_LIST[0], "pwd") == 0 || strcmp(CMD_LIST[0], "cd") == 0 || strcmp(CMD_LIST[0], "exit") == 0 )
	{

		IsBuiltIn = 1;		

	}

	if(IsBuiltIn == 1)
	{
		//Builtin Thing.
		//Start by checking input change. If yes, open file, name specified by GoingForIt[InputChange + 1]. Use fgets to get the param of *InputChange.
		//Otherwise, just do InputChange[0] and InputChange[1]

		//Given InputChange, that will but used as the index. String compare with pwd cd exit
		if( strcmp(CMD_LIST[0], "intro") == 0)
		{
		
			printf(">>>>>>>>>>>>>>> INTRODUCTION <<<<<<<<<<<<<<<\n\n");
			printf("This is 'The Big Shell', by Nicholas Casteen. And yes, that's an MGS2 reference.\n");
			printf("This intro will go over a few need-to-knows, as well as the limitations of the shell.\n");
			printf("Special Thanks to Patrick Lane for his invaluable help in this!\n\n");
			printf("A quick list of commands:\n");
			printf("cd: Changes directory. Can have input from a txt, but has no Output to write to txt.\n");
			printf("pwd: Prints current directory. Output can be sent to a txt.\n");
			printf("exit: Exits the whole shell.\n");
			printf("All non-built in commands should function properly.\n\n");
			printf("Limitations: \n");
			printf("1.) Non-built in commands support output redirection...input redirection ALMOST works? It reads it but...won't take it for some reason?\n");
			printf("2.) This Shell cannot recognize spaces in directory names. \n");
			printf("3.) This Shell cannot use tab or the arrow keys to complete input or scan history. \n");
			printf("4.) This Shell cannot use piping as of right now. \n");
			printf("Most other functions should work properly, including VERY small amounts of error-handling. \n");
			printf("Thank you for reading!\n\n");

		}

		if(strcmp(CMD_LIST[0], "pwd") == 0) //is pwd. Output can be redirected, but takes no inpoot.
		{
			//strcpy(Outpoot, CurrDurr); //Going to put this into a file
			if(OutputChange > 0)
			{
				Reader = fopen(CMD_LIST[OutputChange + 1],"a+");
				fwrite(CurrDurr, 1, strlen(CurrDurr), Reader);
				fclose(Reader);
			}
			else //Going to print it to console
			{
				printf("%s\n", CurrDurr);
			}
			
		}
		else if(strcmp(CMD_LIST[0], "exit") == 0) //is exit
		{
			exit(0);
		}
		else if(strcmp(CMD_LIST[0], "cd") == 0 ) //is cd. THIS IS THE ONLY ONE THAT WILL USE INPUT.
		{
			//STEPS:
			if(InputChange > 0)
			{
				//Use getline()??? Put into Inpoot.
				Reader = fopen(CMD_LIST[InputChange + 1],"r");
				getdelim(&Inpoot,&GetDBuff,'\n',Reader);
				fclose(Reader);
			}
			else //No input change
			{
				//Make Inpoot = to CMD_LIST[1]
				if(CMD_SIZE > 1)
				{
					Inpoot = malloc(strlen(CMD_LIST[1] + 1));
                               		memcpy(Inpoot,CMD_LIST[1],strlen(CMD_LIST[1]));
				}
				else if(CMD_SIZE == 1)
				{	
					
					Inpoot = getenv("HOME");
				}
			}
			
			if(chdir(Inpoot) != 0)
			{
				fprintf(stderr, "Invalid Directory.\n");
			}
			
			
		}
			

	}
	//*********NOTE BUILT IN STUFF*********
	
	else
	{
		//execute not built in stuff
		if(InputChange > 0 && OutputChange > 0)
		{
			fprintf(stderr,"Error: Two redirections cannot occur in the same command.\n");
			InputChange = 0;
			OutputChange = 0;
		}

		/*if(OutputChange > 0) //changed output
		{
			NBICMD_LIST = malloc(sizeof(char*)*OutputChange);
			memcpy(NBICMD_LIST, CMD_LIST, OutputChange); //Puts commands into NBICMD_LIST up to the < or >
		}
		else //No changed output
		{
			NBICMD_LIST = malloc(sizeof(char*)*CMD_SIZE-1);
			printf("About to Copy\n");
			memcpy(NBICMD_LIST, CMD_LIST, CMD_SIZE-1); //JUST COPY THE WHOLE THING IF OUTPUTCHANG == or < 0.
			printf("Copied\n");
		}*/

		ProcID = fork();
		if(ProcID == 0) //child
		{
			
			//if: OutputChange > 0
			//then: fp = freopen(CMD_LIST[OutputChange + 1], "w+', stdout);
			//CMD_LIST[OutputChange] = \0;
			//
			//Okay, idea: Read input of file to an array. Change CMD_LIST[OutputChange] from a pointer to > to a pointer to that array. Make CMD_LIST[[OutputChange + 1] = NULL
			if(OutputChange > 0)
			{
				Reader = freopen(CMD_LIST[OutputChange + 1], "w+", stdout);
				CMD_LIST[OutputChange] = NULL;
			}
			else if(InputChange > 0)
                        {
                                //Use getline()??? Put into Inpoot.
                                Reader = fopen(CMD_LIST[InputChange + 1],"r");
                                getdelim(&Inpoot,&GetDBuff,'\n',Reader);
                                fclose(Reader);
				//And now, we have our input in Inpoot. Now we set CMD_LIST[InputChange] = &Inpoot
				CMD_LIST[InputChange] = Inpoot;
				CMD_LIST[InputChange + 1] = NULL;
                        } 

			if(execvp(CMD_LIST[0],CMD_LIST) < 0) //Uses Command in CMD_LIST and arguments in NBICMD_LIST
			{
				perror("TBS_ERR");

				if(OutputChange > 0)
                        	{
                                	fclose(Reader);
                        	}

				_exit(-1);
			}
			if(OutputChange > 0)
			{
				fclose(Reader);
			}
			_exit(0);
		}

		else if(ProcID < 0) //Error
		{
			perror("TBS_ERR");
		}

		else //Parent
		{
			wait(NULL);
		}
	}

}


int Execyoot(char** CMD_ARR, char* TOK_LIST, int CMDSIZE)
{

        int RedirIN = 0;
        int RedirOUT = 0;
        //FILE* ReadingIt;

        RedirIN = Check_In_Redir(CMD_ARR, CMDSIZE);
        RedirOUT = Check_Out_Redir(CMD_ARR, CMDSIZE);

	
        Capture_Output(CMD_ARR, CMDSIZE, RedirIN, RedirOUT); //check if builtin or not. If RedirIN>0, take input @ TokenLIST[RedirIN+1], ignoring TokenLIST[0+1] if !RedirIN. Otherwise, run TokenLIST[Up to \0]
        return 0;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>!!!!! FINISHED !!!!!<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void Shell_Loop()
{
	int DynaBUFFER = 0; //FOR SOMETHING ELSE
        char* INPT_LIST = NULL;
        char** CMD_LIST; //LIST OF TOKENIZED COMMANDS
	int i; //NEEDED TO PREVENT THOSE DAMN SEGMENT FAULTS
        size_t dirrsize = sizeof(char*)*TKN_BUFFER;
	size_t GetLBuff = 0; //A New Buffer strictly for get line.
	char delimit[]=" \t\r\n\v\f"; //Delimiters
	int status;

	//*************BEGIN LOOP*************
        do
        {
        	getcwd(CurrDurr, dirrsize); //changes CurrDurr. 
		//*************GET THE INPUT*************		
        	while(INPT_LIST == NULL) // TAKE INPUT
        	{
			 printf("$TheBigShell:%s-> ", CurrDurr);
               		 getline(&INPT_LIST,&GetLBuff,stdin); //XXX: EDIT LATER FOR THIS >> AND THAT <<
			 if(strcmp(INPT_LIST, "\n") == 0 || isspace(*INPT_LIST) ) //If the cheeky bugger just hit enter...
			 {
				INPT_LIST=NULL; //Set back to NULL.
			 }
        	}
		

		
		CMD_LIST = malloc(sizeof(char*) * sizeof(INPT_LIST) * TKN_BUFFER); //MAKE CMD_LIST AS BIG AS INPT_LIST, AS THAT'S THE LARGET IT CAN BE

		i=0; //reset i. WE'LL NEED THIS.

		
		//*************TOKENIZE ALL THE INPOOT*************
        	CMD_LIST[i]=strtok(INPT_LIST,delimit); //Do the first one
		DynaBUFFER = sizeof(INPT_LIST);
        	while(CMD_LIST[i] != NULL) //Eventually, strtok will give a NULL, once it reaches the end of the inpoot.
        	{

			if(i >= DynaBUFFER) //Position above or equal to the Buffer.
                	{

                        	DynaBUFFER += TKN_BUFFER; //Add by a factor of TokeBuffer
                        	CMD_LIST=realloc(CMD_LIST, sizeof(char*)*DynaBUFFER); //Reallocate Memory. Returns a NEW pointer addresss.

                	}

                	i++;
                	CMD_LIST[i]=strtok(NULL,delimit); //strtok is storing its own data, the cheeky cunt. Stores the last string it was looking at. NULL means it keeps doing that string.
                                                    //So...careful with strtok! It's a global function, I guess????
        	}
		
       		status = Execyoot(CMD_LIST, INPT_LIST, i);
		
        	INPT_LIST = NULL; //Empty Input List
        	CMD_LIST = NULL; //Empty Command
		

        }
        while(1);

}

int main(int argc, char **argv)
{
	printf("\n\n\n\n");
	printf("****************************************** THE BIG SHELL ******************************************\n");
	printf("************************************ CODED BY NICHOLAS CASTEEN ************************************\n");
	printf("************************************ TYPE 'intro' FOR MORE INFO ***********************************\n\n\n");
	CurrDurr = malloc(sizeof(char*)*TKN_BUFFER);
        Shell_Loop();
        return 1;
}

