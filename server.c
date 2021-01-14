/* 
This code primarily comes from 
http://www.prasannatech.net/2008/07/socket-programming-tutorial.html
and
http://www.binarii.com/files/papers/c_sockets.txt
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

///global var for CPU
double CPU=0.0;
double max = 0.0;
int end_funct = 0;
pthread_mutex_t lock;
int total_CPU = 0;
double seconds = 0.0;
double av_CPU = 0.0;

#define PORT_NUM 3002

//function that does part1
void *calCPU(void *file){
    char liner[255];
    char* line = liner;
    int counter = 0;
    double last_idle = 0.0;
    //create a file pointer and open a file
    FILE *fptr;
    
    //while loop to execute the program
    while(1){
        if ((fptr = fopen("/proc/stat", "r")) == NULL) {
            printf("Error! opening file");
            // Program exits if file pointer returns NULL.
            exit(1);
        }
        //check if it's time to quit the function 
        pthread_mutex_lock(&lock);
        if(end_funct){
            pthread_mutex_unlock(&lock);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&lock);
        counter = 0;
        
        fgets(line,255,fptr);
        char line_copy[strlen(line)+1];         //creating a copy of the line 
        strcpy(line_copy, line);                //creating a copy of the line 
        //printf("%s\n",line);
        const char delim[4] = " ";            //initializing delimeters
        char* token = strtok(line_copy,delim);
        //keep tokenizing until we retrieve the fourth number (total amount of idle time)
        while(counter!=4){
            token = strtok(NULL,delim);
            counter++;
        }
        
        
        double curr_idle =  atof(token);
        //printf("%f\n",curr_idle);
        
        double difference = curr_idle-last_idle;
        double curr_CPU = 100-(difference)/4.00;
        if(curr_CPU<0){
            curr_CPU=0;
            //printf("%f\n",0);
        }
        else if(curr_CPU>100){
            curr_CPU=100;
            //printf("%f\n",100);
        }
        pthread_mutex_lock(&lock);
        CPU=curr_CPU;
        //printf("%f\n",curr_CPU);
        //updating max
        if(curr_CPU>max){
            max = curr_CPU;
        }
        //add up to the total sum 
        total_CPU+=curr_CPU;
        //update the seconds count
        seconds++;
        if(seconds==3600){
            total_CPU = 0;
            seconds=0;
        }
        //calculate the average
        double average_CPU = total_CPU/seconds;
        //update global var for average
        av_CPU = average_CPU;
        pthread_mutex_unlock(&lock);
        last_idle = curr_idle;
        
        //close the file
        fclose(fptr);
        sleep(1);
        
  }
    //return 0;
}

void *start_server(void *p)
{
    //initialize treshold value 
      // structs to represent the server and client
      struct sockaddr_in server_addr,client_addr;    
      
      int sock; // socket descriptor

      // 1. socket: creates a socket descriptor that you later use to make other system calls
      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
      }
      int temp;
      if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&temp,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
      }

      // configure the server
      server_addr.sin_port = htons(PORT_NUM); // specify port number
      server_addr.sin_family = AF_INET;         
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8); 
      
      // 2. bind: use the socket and associate it with the port number
      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
      }

      // 3. listen: indicates that we want to listen to the port to which we bound; second arg is number of allowed connections
     if (listen(sock, 1) == -1) {
         perror("Listen");
         exit(1);
      }
          
      // once you get here, the server is set up and about to start listening
      printf("\nServer configured to listen on port %d\n", PORT_NUM);
      fflush(stdout);
     
    //int count = 0; // count the number of pages requested (for debugging purposes)
    int treshold = 101;
    while(1) { // keep looping and accept additional incoming connections
        
      // 4. accept: wait here until we get a connection on that port
      int sin_size = sizeof(struct sockaddr_in);
      int fd = accept(sock, (struct sockaddr *)&client_addr,(socklen_t *)&sin_size);
      if (fd != -1) {
         printf("Server got a connection from (%s, %d)\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        // buffer to read data into
        char request[1024];
          
        //printf("%s\n",request);

        // 5. recv: read incoming message (request) into buffer
        int bytes_received = recv(fd,request,1024,0);
        // null-terminate the string
        request[bytes_received] = '\0';
        // print it to standard out
        //printf("REQUEST:\n%s\n", request);
        char *token = NULL;
        token = strtok(request,"=");
          //tokenizing the request to receive the users input
          int counter = 2;
          while(counter>0){
            token = strtok(NULL, "=");
            counter--;
          }
          //string to a number
          treshold = atoi(token);

        //count++; // increment counter for debugging purpose
          // this is the message that we'll send back
        char* response = (char*)malloc(5000 * sizeof(char));
        pthread_mutex_lock(&lock);
        //break the while loop if user entered "q"
        double CPU_copy = CPU;
        double max_copy = max;
        double av_CPU_copy= av_CPU;
        pthread_mutex_unlock(&lock);
        
          
        //sprintf(response,"HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n<html><p>JUSTPUTITHERE:<br>count=%f</p></html>");
        if(CPU_copy>treshold&&treshold!=0){ 
        sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
                "<html>"
                "<meta charset=\"UTF-8\">"
                "<p style=\"color:#FF0000\">Latest CPU:<br>count=%f</p>"
                "<p>Max CPU:<br>count=%f</p>"
                "<p>Average CPU:<br>count=%f</p>"
                "<form method = \"GET\"action=\"=\">"
                "<label for=\"treshold\">Please input your treshold:</label>"
                "<br><input type=\"text\" id=\"treshold\" name=\"treshold\"><br>"
                "<button type=\"submit\" onclick=\"submit()\"> Submit </button></form>"
                "<script type=\"text/javascript\">"
                "window.alert(\"Uh-oh!Treshold exceeded!\");"
                "</script>"
                "</body></html>", CPU_copy, max_copy,av_CPU_copy);
        }
        else{
              sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
                "<html>"
                "<meta charset=\"UTF-8\">"
                "<body><p>Latest CPU:<br>count=%f</p>"
                "<p>Max CPU:<br>count=%f</p>"
                "<p id=\"CPU\">Average CPU:<br>count=%f</p>"
                "<form method = \"GET\"action=\"=\">"
                "<label for=\"treshold\">Please input your treshold:</label>"
                "<br><input type=\"text\" id=\"treshold\" name=\"treshold\"><br>"
                "<button type=\"submit\" onclick=\"submit()\"> Submit </button></form>"
                "<script type=\"text/javascript\">"
                "</script>"
                "</body></html>", CPU_copy, max_copy,av_CPU_copy);
        }
        
        //printf("RESPONSE:\n%s\n", response);
          
        
        // 6. send: send the outgoing message (response) over the socket
        // note that the second argument is a char*, and the third is the number of chars	
        send(fd, response, strlen(response), 0);

        free(response);
        
        // 7. close: close the connection
        close(fd);
        printf("Server closed connection\n");
       }
    }

    // 8. close: close the socket
    close(sock);
    printf("Server shutting down\n");
  
    return 0;
} 
//listens for the signal to quit the function 
void *listen_quit(void *p){
    char input_val[100];
    while(1){
        scanf("%s", input_val);
        if(strcmp(input_val,"q")==0){
            pthread_mutex_lock(&lock);
            end_funct = 1;
            pthread_mutex_unlock(&lock);
            pthread_exit(NULL);
        }
    }
    
}

int main(int argc, char *argv[])
{
    pthread_mutex_init(&lock,NULL);
    /*
  // check the number of arguments
  if (argc != 2) {
      printf("\nUsage: %s [port_number]\n", argv[0]);
      exit(-1);
  }

  int port_number = atoi(argv[1]);
  if (port_number <= 1024) {
    printf("\nPlease specify a port number greater than 1024\n");
    exit(-1);
  }*/
    //initialize pthreads
    pthread_t t1,t2,t3;
    
//   int port_number = 3000; // hard-coded for use on Codio
    int iret1 = pthread_create(&t1, NULL, start_server, NULL);
    if(iret1!=0){
        return -1;
    }
    int iret2 = pthread_create(&t2,NULL,calCPU, NULL);
    if(iret2!=0){
        return -1;
    }
    int iret3 = pthread_create(&t3,NULL,listen_quit,NULL);
    if(iret3!=0){
        return -1;
    }
    pthread_join(t3,NULL);
    pthread_join(t2,NULL);
    pthread_cancel(t1);
    pthread_join(t1,NULL);
    printf("Shutting down\n");
    //int iret2 = pthread_create(&t2, NULL, calCPU , argv[1]);
   //start_server(port_number);
   //int thread = pthread_create(&t2,NULL,print,NULL);
   exit(0);
//   return 0;
}

