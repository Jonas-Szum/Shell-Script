#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <fcntl.h>
//using namespace std;

//typedef void (*sighandle)(int);
//sighandle *signal(int num, sighandle* typeHandle);
const int COMMAND_SIZE = 100;
const int COMMAND_NUM = 20;
const int LINE_NUM = 500;
char* FILENAME = "output.txt";
char* FILENAME2 = "output2.txt";
void signal_handle(int num)
{
if(num == SIGINT)
  {
  write(0, "caught sigint\n", 14);
  write(0, "CS361 > ", 9);
  }
else
  {
  write(0, "caught sigstp\n", 14);
  write(0, "CS361 > ", 9);
  }
}

void removeSpaces(char* myStr)
{
int index = 0;
int i;

while(myStr[index] == ' ' || myStr[index] == '\t') //ignore the leading spaces
  index++;

if(index != 0) //copy the rest of the string like normal
  {
  i = 0;
  while(myStr[i + index] != '\0')
    {
    myStr[i] = myStr[i + index];
    i++;
    }
  myStr[i] = '\0';
  }
}

void command(char line[LINE_NUM])
{
char* pipedCommands = strtok(line, "|");
int readFromFile = 0;
int fileDesc = -1;
int status; //status is what the child returns to the parent
int i = 0;  //i is used as debugging to prevent fork bombs
char finalPrint[LINE_NUM*2];
finalPrint[0] = '\0';

while(pipedCommands)
  {
  char myCmds[LINE_NUM];
  strcpy(myCmds, pipedCommands); //copy current command to myCmds
  pipedCommands = strtok(NULL, "\n"); //store the rest of the command in pipedCommands
  removeSpaces(myCmds);
  char *word = strtok(myCmds, " "); //word becomes a single command with arguments
  int j = 0;			    //j will be used with argsarray
  char* argsarray[COMMAND_NUM];	    //used with j to keep track of arugments
  while (word) {
    //copy a word to the arg array
    argsarray[j] = strdup(word);    //store each argument
    //get next word
    word = strtok(NULL, " ");       //keep going until we're at the end of the string
    j = j + 1;
    }

  if(readFromFile == 1) //read from the file that was used as input last time
    {
    if(i%2 == 1)
      argsarray[j] = FILENAME;
    else
      argsarray[j] = FILENAME2;
    //close(fileDesc);
    }
  int pid = fork();

  if (pid == 0) //execute the command as given by the user
    {
    if (pipedCommands != NULL)
      {
      if(i%2 == 0) //i gets incremented after this call
        {
        remove(FILENAME);
        fileDesc = open(FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        }
      else
        {
        remove(FILENAME2);
        fileDesc = open(FILENAME2, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        }
      dup2(fileDesc, 1); //replace stdout with the new file
      close(fileDesc);
      }
    else
      {
      dup2(1, 1);
      }
    int val = execvp(myCmds, argsarray);
    exit(val);
    }
  else
    {
    dup2(1, 1);
    wait(&status);
    if (pipedCommands != NULL)
      {
      char buf[LINE_NUM];
      snprintf(buf, 13+2*sizeof(int), "pid:%d status:%d\n", pid, WEXITSTATUS(status));
      readFromFile = 1;
      strcat(finalPrint, buf);
      }
    else
      {
      readFromFile = 0;
      if(i > 0)
        printf("%s", finalPrint);
      printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
      }
    }
  pipedCommands = strtok(pipedCommands, "|"); //continue with the pipeline
  i++;
  }

}

int main()
{
signal(SIGINT, signal_handle);
signal(SIGTSTP, signal_handle);

while(1)
  {
  char line[LINE_NUM];

  write(0, "CS361 > ", 9);
  fgets(line, 500, stdin);
  line[strlen(line)-1] = '\0';
  if(strcmp(line, "exit") == 0) break;
  else
    {
    char *word = strtok(line, ";"); //save each chain of commands that ends in ;
    while (word) {
      char tempWord[LINE_NUM];
      strcpy(tempWord, word);
      word = strtok(NULL, "\n"); //prepare the next string of commands
      command(tempWord); //process current string of commands
      word = strtok(word, ";");
      }
    }
  }
//remove(FILENAME);
//remove(FILENAME2);
return 0;
}
