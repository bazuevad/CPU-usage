#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//calculate the CPU usage 
//

//global var for CPU
double CPU;
int idle_time;

//function that does part1
int main(int argc, char* argv[]){
    char* line[255];
    int round = 0;
    int counter = 0;
    //create a file pointer and open a file
    FILE *fptr;
    
    //while loop to execute the program
    while(1){
        counter = 0;
        if ((fptr = fopen(argv[1], "r")) == NULL) {
            printf("Error! opening file");
            // Program exits if file pointer returns NULL.
            exit(1);
        }
        fgets(line,255,fptr);
        char line_copy[strlen(line)+1];         //creating a copy of the line 
        strcpy(line_copy, line);                //creating a copy of the line 
        const char delim[4] = " ";            //initializing delimeters
        char* token = strtok(line_copy,delim);
        //keep tokenizing until we retrieve the fourth number (total amount of idle time)
        while(counter!=4){
            token = strtok(NULL,delim);
            counter++;
        }
        //first time running this loop 
        if(round==0){
            int num = atoi(token);
            printf("%d\n",num);
            idle_time = num;
        }
        //if this is not first time running this loop
        if(round>0){
            //do a calculation 
            double curr_CPU = 100-(atoi(token) - idle_time)/4;
            CPU = curr_CPU;
            int num = atoi(token);
            idle_time = num;
            if(curr_CPU<0){
                printf("%d\n",0);
            }
            else if(curr_CPU>100){
                printf("%d\n",100);
            }
            else{
            printf("%f\n",curr_CPU);
            }
        }
        //sleep for 2 seconds
        sleep(1);
        //close the file
        fclose(fptr);
        round++;
  }
    return 0;
}