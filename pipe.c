#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

        // setting up a 2D array for the file descriptors of all the pipes 
        int fds[argc-1][2];
        // set up array for child process ids
         pid_t cpid[argc-1], w;
         int wstatus;

         int fail_flag = 0, e = 0;

         //no programs as comman line arguments 
         if (argc < 2) {
           exit(EINVAL);
         }

        // setting up the pipes 
        for (int i = 0; i < argc-1; i++) {
           if (pipe(fds[i]) == -1) {
                perror("Pipe");
                exit(errno);
           }
        }
     
        for (int j = 0; j < argc-1; j++) {
           cpid[j]= fork();
           if (cpid[j] < 0) {
              perror("Fork");
              exit(errno);
           } else if (cpid[j] == 0 ) {
              //for all commands except the last
              if ( j < argc -2 ) {
                   if (dup2(fds[j][1], STDOUT_FILENO) < 0) {
                      perror("dup21");
                      exit(errno);
                   }
              }
              //for all commands excexpt the first
              if (j !=  0) {
                  if (dup2(fds[j-1][0], STDIN_FILENO) < 0) {
                      perror("dup22");
                      exit(errno);
                  }
              }
              for (int l = 0; l < argc-1; l++)  {
               close(fds[l][0]); 
               close(fds[l][1]); 
              }
              if (execlp(argv[j+1],argv[j+1],NULL) < 0 ) {
                 exit(errno);
              }
           } else {
              close(fds[j][1]);
              if (j != 0) {
                 close(fds[j-1][0]);
              }
             do {
               w = waitpid(cpid[j], &wstatus, WUNTRACED);
               if (w == -1) {
                   perror("waitpid");
                   exit(errno);
               } 
                if (WIFEXITED(wstatus)) {
                    if (WEXITSTATUS(wstatus) != 0 && fail_flag == 0) {
                        fail_flag=1; 
                        close(fds[j][0]); 
                        e = WEXITSTATUS(wstatus);
                       exit(e);
                    }
                }
            } while(!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus)); 
           }
        }

  

      close(fds[argc-2][0]); 
      if (fail_flag == 1) exit(e);
      else exit(EXIT_SUCCESS);
}

