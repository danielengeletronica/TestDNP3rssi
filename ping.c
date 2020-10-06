#include <ctime>
#include <sys/time.h>
#include <climits>
#include<stdio.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<system.h>
#include <unistd.h>
#include <time.h> 
double t;
double t0;
double t1;

double getCurrentRealTimer(void)
    {
        struct timeval t;
        gettimeofday(&t, 0);
        return t.tv_sec + t.tv_usec * 1.0e-6;
    }

int main()
{
	
	FILE *pont_arq; // cria variável ponteiro para o arquivo
	float timestamp; // variável do tipo string
	char n[2] = "\n";
	char v[2] = ",";
	int i;
	double dt;
	double dt1;
 pont_arq = fopen("ping.txt", "a");
  
  //testando se o arquivo foi realmente criado
  if(pont_arq == NULL)
  {
  printf("Erro na abertura do arquivo!");
  return 1;
  }
  
 t=getCurrentRealTimer();
 for (i = 0;i<10;i++)
 { 
	 t0=getCurrentRealTimer();
	 if ( system(“ping -c1 192.168.1 -w 2 “) == 0)
			t1=getCurrentRealTimer();
			dt = t1-t0;
			dt1 = t1-t;
			dt = dt*1000;
			dt1 = dt1*1000;
			fprintf(pont_arq, "%f", dt);
			fprintf(pont_arq, "%s", v);
			fprintf(pont_arq, "%f", dt1);
			fprintf(pont_arq, "%s", n);
	else 
			t1=getCurrentRealTimer();
			dt = 200000000000000;
			dt1 = t1-t;
			fprintf(pont_arq, "%f", dt);
			fprintf(pont_arq, "%s", v);
			fprintf(pont_arq, "%f", dt1);
			fprintf(pont_arq, "%s", n);
			
	usleep(500000);
 }
 
 
	fclose(pont_arq);
	
	return 0;
}