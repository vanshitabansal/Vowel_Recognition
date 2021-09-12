// Assignment-2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "ctype.h"
#include<stdlib.h>
#include "util.h"    //this is the header file containing all the functions used while prediction
#include <math.h>
#include<float.h>
#define p 12  
#define N 320
char buffer[400];
FILE *file_input, *file_output,*ref,*test,*normalised;

int _tmain(int argc, _TCHAR* argv[])
{
	printf("This is vowel recognition assignment (214101056) \n");
	printf("Please wait..The processing is going on! \n");
	printf("---------------------------------------------------------------------------------\n");
	//Declaring all the required arrays and variables

	char arr[2048],ch,vowel[]={'a','e','i','o','u','\0'};
	double data[10][5][320],r[10][5][13],A[10][5][12],C[10][5][13],tokhura[5]={0},Average_C[5][12]={{0}},num=0,energy[100000]={0},max=0,e=0,sine_window=0,correct_pred=0;
	double tokhura_weights[12]={1.0,3.0,7.0,13.0,19.0,22.0,25.0,33.0,42.0,50.0,56.0,61.0};
	int cnt=0,framecount=0,count=0,position=0,index=0,sample=0;
	double alpha[13][13],k[13],E[13];
		
	//Training first 10 utterance of each vowel to create reference file

	for(int v=0;v<5;v++){
		for(int vowel_no=0;vowel_no<10;vowel_no++){	
			max=0;
			sample=0;
			framecount=0;	

			sprintf(buffer, "214101056_%c_%d.txt",vowel[v],vowel_no+1);		
			fopen_s(&file_input,buffer,"r");
			fopen_s(&normalised,"Normalised_test.txt","a+");
		
			//Normalising the data by calling this function by correcting dc shift 
			// multiplying by normalisation factor
			normalisation(file_input,normalised);


			//For calculating steady part we will calculate the maximum energy frame and then take 2 frames from its left and two from its right (total 5 frames)

			rewind(normalised);
			ch=fgetc(normalised);
			while(!feof(normalised)){			
				while(ch!='\n' ){
					arr[index++]=ch;
					ch=fgetc(normalised);	
				}
				arr[index++]='\0';		
				num=atof(arr);
				index=0;			
				if(count==320){
					energy[framecount]=e/320.0;
					if(energy[framecount]>max){
						position=framecount;			//position variable store the frame number of maximum energy frame
						max=energy[framecount];
					}
					framecount++;
					e=0;
					count=0;
				}
				e+=num*num;
				count++;		
			ch=fgetc(normalised);		
		}

			//Reaching to position in normalised data file

			position=position-2;
			rewind(normalised);
			ch=fgetc(normalised);
			index=0,framecount=0,count=0;
			while(!feof(normalised)){
				while(ch!='\n' ){
					arr[index++]=ch;
					ch=fgetc(normalised);	
				}
				arr[index++]='\0';		
				num=atof(arr);
				index=0;
				if(count==320){
					framecount++;
					count=0;
				}
				count++;
				if(framecount==position){
					break;
				}			
			ch=fgetc(normalised);		
			}

			//Storing these 5 frames in array data[10][5][320] vowel by vowel

			ch=fgetc(normalised);
			sample=0,index=0;
			for(int frame=0;frame<5;){
				while(ch!='\n' ){
					arr[index++]=ch;
					ch=fgetc(normalised);	
				}
				arr[index++]='\0';		
				num=atof(arr);
				index=0;
				data[vowel_no][frame][sample] = num;
				if(sample==319){
					frame++;
					sample=0;
				}
				if(frame>4)
					break;
				
				sample++;
				ch=fgetc(normalised);	
			}

			//Applying Hamming Window on data
			apply_hamming_window(data,vowel_no);
		
			//Calculating Ri's
			calculate_ri(data,r,vowel_no);
		
			//Calculating Ai's
			calculate_ai(r,A,alpha,k,E,vowel_no);

			//Calculating Ci's
			calculate_ci(r,A,C,vowel_no);
	
			//Applying raised sine window on Ci's 
			apply_raised_sine_window(C,vowel_no);

			fclose(file_input);
			fclose(normalised);
			remove("Normalised_test.txt");
		}

	//Calculating average of Ci's per frame and writing it in reference file for particular vowel
	calculate_avg_ci(vowel,v,Average_C,C,file_output);
	
	
	}
	printf("Reference files have been generated! \n");

	//Testing the remaining 10 vowel utterances (11-20) by creating test files first
	for(int v=0;v<5;v++){
		for(int vowel_no=0;vowel_no<10;vowel_no++){	
			max=0;
			sample=0;
			framecount=0;  
			sprintf(buffer, "214101056_%c_%d.txt",vowel[v],vowel_no+11);   //11 to start from utterance 11 (0+11)
			fopen_s(&file_input,buffer,"r");
			fopen_s(&normalised,"Normalised_test.txt","a+");
		
			//Normalising the data by calling this function by correcting dc shift 
			// multiplying by normalisation factor
			normalisation(file_input,normalised);	

			//Same process as above
			rewind(normalised);
			ch=fgetc(normalised);
			while(!feof(normalised)){			
				while(ch!='\n' ){
					arr[index++]=ch;
					ch=fgetc(normalised);	
				}
				arr[index++]='\0';		
				num=atof(arr);
				index=0;			
				if(count==320){
					energy[framecount]=e/320.0;
					if(energy[framecount]>max){
						position=framecount;
						max=energy[framecount];
					}
					framecount++;
					e=0;
					count=0;
				}
				e+=num*num;
				count++;		
				ch=fgetc(normalised);		
			}

			position=position-2;
			rewind(normalised);
			ch=fgetc(normalised);
			index=0,framecount=0,count=0;
			while(!feof(normalised)){
				while(ch!='\n' ){
					arr[index++]=ch;
					ch=fgetc(normalised);	
				}
				arr[index++]='\0';		
				num=atof(arr);
				index=0;
				if(count==320){
					framecount++;
					count=0;
				}
				count++;
				if(framecount==position){
					break;
				}			
			ch=fgetc(normalised);		
			}

			ch=fgetc(normalised);		
			sample=0,index=0;
			for(int frame=0;frame<5;){
				while(ch!='\n' ){
					arr[index++]=ch;
					ch=fgetc(normalised);	
				}
				arr[index++]='\0';		
				num=atof(arr);
				index=0;
				data[vowel_no][frame][sample] = num;
				if(sample==319){
					frame++;
					sample=0;
				}
				if(frame>4)
					break;
				sample++;
				ch=fgetc(normalised);	
			}
		
			apply_hamming_window(data,vowel_no);
		
			calculate_ri(data,r,vowel_no);
		
			calculate_ai(r,A,alpha,k,E,vowel_no);

			calculate_ci(r,A,C,vowel_no);
	
	
			//Apply raised sine window on Ci's and creating test file for each utterance
			for(int frame=0;frame<5;frame++){
				sprintf(buffer, "test_%c_%d.txt",vowel[v],vowel_no+11);		
				file_output=fopen(buffer,"a+");		
				for(int j=1;j<=12;j++){
					sine_window=1+(12*1.0/2)*sin(3.14*j/12);
					C[vowel_no][frame][j]*=sine_window;
					fprintf(file_output,"%lf\n",C[vowel_no][frame][j]);
				}		
				fclose(file_output);
			}

		fclose(file_input);
		fclose(normalised);
		remove("Normalised_test.txt");	
		}	
	}

	//Taking test file one by one for each vowel and calculating tokhura's distance from each reference files(5)
	for(int v=0;v<5;v++){
		for(int t=11;t<=20;t++){
			double min=DBL_MAX,average_tokhura=0;
			char pred;
			sprintf(buffer, "test_%c_%d.txt",vowel[v],t);		
			fopen_s(&test,buffer,"r");
			for(int i=0;i<5;i++){
				sprintf(buffer, "reference_%c.txt",vowel[i]);		
				fopen_s(&ref,buffer,"r");

				//calculating tokhura's distance
				tokhura[i]=find_tokhura_distance(tokhura_weights,test,ref);
				fclose(ref);
			}
			for(int i=0;i<5;i++){
				if(tokhura[i]<min){
				min=tokhura[i];
				pred=vowel[i];				//storing the vowel that has minimum tokhura's distance
				}
			}
			
			printf("The Tokhura distance between %c and all the vowel :\n\n",vowel[v]);
			for(int i=0;i<5;i++)
				printf("\t %c is : %lf\n",vowel[i],tokhura[i]);
			printf("\nTHE VOWEL IS : %c \nYOUR PREDICTION IS : %c\n\n",vowel[v],pred);
			printf("---------------------------------------------------------------------------------\n\n");
			if(vowel[v]==pred)
				correct_pred++;
			fclose(test);
		}
		
		
	}
	removeAllFiles();
	printf("The accuracy obtained is : %.2lf percent \n\n",correct_pred*100.0/50.0);
	
	
	return 0;
}

