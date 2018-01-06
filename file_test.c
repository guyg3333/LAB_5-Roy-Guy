/*
 ============================================================================
 Name        : file_test.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
int main(void) {


	 FILE *fd;
	 int i,r,num;
	 char buffer[300] ;



	//puts("!!!Hello World!!! \n"); /* prints !!!Hello World!!! */


	 num = 10;
	 fd = fopen("alice.txt", "r");

	 if(fd ==NULL)
	 {
	    	 perror("file open");
	    	 exit(1);
	 }

	 //print("%s","hi hi hi hi");
	 /*
	 if(fprintf(fd,"%s",buffer)==-1)
	  {
		  perror("file");
		      exit(1);
	  }
	  */





	 r = atoi ("3451");
	 printf("%d \n\n\n\n",r);

	 while(num)
	 {

	 for(i=0;i<1024;i++)
		 {
		 buffer[i] = fgetc(fd);
		 if( feof(fd) ) {
				 num =0;
				  break ;

			   }
		 }
	for(i=0;i<1024;i++)
		printf("%c",buffer[i]); /* prints !!!Hello World!!! */
	 num--;
	 }


	fclose(fd);

	//Guy
	return EXIT_SUCCESS;


}
