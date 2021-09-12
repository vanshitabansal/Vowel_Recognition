// util.h : include file for project specific files
//that are used frequently, but are changed infrequently
#include "stdafx.h"
#include "stdio.h"
#include "ctype.h"
#include<stdlib.h>
#include<math.h>

#define p 12
#define N 320
void apply_hamming_window(double data[10][5][320],int vowel_no){
	double weight=0;
	for(int frame=0;frame<5;frame++){
			for(int j=0;j<N;j++){
				weight=(0.54-0.46*cos(2*3.14*j/(N-1)));
				data[vowel_no][frame][j]*=weight;
			}
		}

}
void calculate_ri(double data[10][5][320],double r[10][5][13],int vowel_no){
		for(int frame=0;frame<5;frame++){
			for(int i=0;i<=12;i++){
				r[vowel_no][frame][i]=0;
				for(int j=0;j<=N-i-1;j++){
					r[vowel_no][frame][i]+=data[vowel_no][frame][j]*data[vowel_no][frame][i+j];
				}
			}
		}
}
void calculate_ai(double r[10][5][13],double A[10][5][12],double alpha[13][13],double k[13],double E[13],int vowel_no){
	double summation=0;
	for(int frame=0;frame<5;frame++){
		E[0]=r[vowel_no][frame][0];
		for(int i=1;i<=12;i++){
			summation=0;
			for(int j=1;j<=i-1;j++){
				summation+=alpha[i-1][j]*r[vowel_no][frame][i-j];
			}		
		k[i]=(r[vowel_no][frame][i]-summation)*1.0/E[i-1];		
		alpha[i][i]=k[i];
		for(int j=1;j<=i-1;j++)
			alpha[i][j]=alpha[i-1][j]-k[i]*alpha[i-1][i-j];
		E[i]=(1-k[i]*k[i])*E[i-1];
		}
		for(int i=1;i<=12;i++){
			A[vowel_no][frame][i-1]=alpha[12][i];
		}
	}
		
}
void calculate_ci(double r[10][5][13],double A[10][5][12],double C[10][5][13],int vowel_no){
	double sigma=0,summation=0;
	for(int frame=0;frame<5;frame++){
		sigma=r[vowel_no][frame][0];
		C[vowel_no][frame][0]=(log(sigma*sigma)/log(10.0));
		for(int i=1;i<=12;i++){
			summation=0;
			for(int k=1;k<=i-1;k++){
				summation+=((k*1.0/i)*C[vowel_no][frame][k]*A[vowel_no][frame][i-k-1]);
			}
			C[vowel_no][frame][i]=A[vowel_no][frame][i-1]+summation;
		}
	}	
}
void apply_raised_sine_window(double C[10][5][13],int vowel_no){
	double sine_window=0;
	for(int frame=0;frame<5;frame++){
		for(int j=1;j<=12;j++){
			sine_window=1+(12*1.0/2)*sin(3.14*j/12);
			C[vowel_no][frame][j]*=sine_window;
		}		
	}
}
void calculate_avg_ci(char vowel[5],int v,double Average_C[5][12],double C[10][5][13],FILE *file_output){
	
	char buffer2[400];
	sprintf(buffer2, "reference_%c.txt",vowel[v]);
	fopen_s(&file_output,buffer2,"a+");
		
	for(int frame=0;frame<5;frame++){		
		double sum=0;
		int j=0;
		for(j=1;j<=12;j++){		
			for(int vowel_no=0;vowel_no<10;vowel_no++){
				Average_C[frame][j-1]+=C[vowel_no][frame][j];	
			}
			Average_C[frame][j-1]=Average_C[frame][j-1]/10.0;			
		}		
	} 
	for(int frame=0;frame<5;frame++){
		for(int j=0;j<12;j++){
				fprintf(file_output,"%lf\n",Average_C[frame][j]);
		}
	}
	fclose(file_output);
}
double find_tokhura_distance(double tokhura_weights[12],FILE* test,FILE* ref){
	double Cr[5][12],Ct[5][12],num=0,sum=0,total_sum=0;	
	char arr[1024];
	int index=0,frame=0,j=0;
	rewind(test);
	rewind(ref);

	//Reading reference and test files and saving Ci's in array Cr and Ct
	char ch=fgetc(test);
	while(!feof(test)){			
		while(ch!='\n' ){
				arr[index++]=ch;
				ch=fgetc(test);	
			}
			arr[index++]='\0';		
			num=atof(arr);
			index=0;
			Ct[frame][j++]=num;
			if(j==12){
				frame++;
				j=0;
			}			
			ch=fgetc(test);
	}
	index=0,frame=0,j=0;
	ch=fgetc(ref);
	while(!feof(ref)){			
		while(ch!='\n' ){
				arr[index++]=ch;
				ch=fgetc(ref);	
			}
			arr[index++]='\0';		
			num=atof(arr);
			index=0;
			Cr[frame][j++]=num;
			if(j==12){
				frame++;
				j=0;
			}
			
			ch=fgetc(ref);
	}

	//Calculating Tokhura's distance using weight array
	for(int i=0;i<5;i++){
		sum=0;
		for(int k=0;k<12;k++){
			sum+=tokhura_weights[k]*(Ct[i][k]-Cr[i][k])*(Ct[i][k]-Cr[i][k]);
		}
		total_sum+=sum;
	}
	return total_sum;
}
double DC_shift(FILE *file_input){

	//Calculating DC shift 
	char ch;
	char arr[10];
	ch=fgetc(file_input);
	int index=0;
	double dcshift=0;
	double sample_count=0;
	while(!feof(file_input)){

		while(ch!='\n'){
			arr[index++]=ch;			
			ch=fgetc(file_input);	
		}

		arr[index]='\0';
		sample_count++;
		dcshift+=atoi(arr);
		index=0;
		ch=fgetc(file_input);	
	}
	
	dcshift/=sample_count*1.0;        //Calculation DC shift is done as:  summation of dc shift values divided by number of samples.
	
	//printf("DC Shift : %lf \n",dcshift);
	return dcshift;

}
double normalisation_factor(FILE *file_input){
		
	//Calculating Normalization factor taking limit as + and - 5000
	char ch;
	double num=0;
	char a[1600];
	double max=0;
	ch=fgetc(file_input);
	int index=0;
	double factor=0;
	while(!feof(file_input)){
		while(isalpha(ch)){             //ignoring the header information in sample data, 
			while(ch!='\n'){			//This can be skipped by saving data without header information also.
			ch=fgetc(file_input);
			}
			ch=fgetc(file_input);
		}
		while(ch!='\n' ){
			a[index++]=ch;
			ch=fgetc(file_input);	
		}
		a[index]='\0';				//reading characters and storing in array then converting it to integer
		num=abs(atof(a));
		if(max<num)
			max=num;
		index=0;
		ch=fgetc(file_input);		
	}
	if(max<5000){
		factor=1.0;
	}
	else
		factor=(max-5000.0)/(max*1.0);					//Calculation of normalisation factor using maximum absolute amplitude
	//printf("Maximum Amplitude : %lf \n",max);
	//printf("Normalisation Factor : %lf \n",factor);
	//printf("---------------------------------------\n");
	return factor;
	
}

void normalisation(FILE *file_input,FILE *file_output){

	FILE * input;
	fopen_s(&input,"dc_shift.txt","r");
	double dcshift=DC_shift(input);
	fclose(input);
	double factor=normalisation_factor(file_input);

	//Normalising the  given data by subtracting DC shift and multipying normalisation factor
			
	char ch;
	double num=0;
	char ar[1600];
	int index=0;
	rewind(file_input);
	ch=fgetc(file_input);	
	while(!feof(file_input)){
		while(isalpha(ch)){							//ignoring the header information in sample data, 
			while(ch!='\n'){						//This can be skipped by saving data without header information also.
				ch=fgetc(file_input);
			}
			ch=fgetc(file_input);
		}
		while(ch!='\n' ){
			ar[index++]=ch;
			ch=fgetc(file_input);	
		}
		ar[index++]='\0';
		num=atof(ar);
		index=0;
		num=(num-dcshift)*factor;							//Correcting dc shift by subtracting it with each sample data and normalising it to +/-5000.
			
		fprintf(file_output,"%lf\n",num); 		
		ch=fgetc(file_input);
		
	}
	
}
//This function is made to remove the temporary files created during execution as there was some technical issue in file handling process.
void removeAllFiles(){
	remove("reference_a.txt");	remove("reference_e.txt");	remove("reference_i.txt");	remove("reference_u.txt");	remove("reference_o.txt");	
	remove("test_a_11.txt");remove("test_a_12.txt");remove("test_a_13.txt");remove("test_a_14.txt");remove("test_a_15.txt");
	remove("test_a_16.txt");remove("test_a_17.txt");remove("test_a_18.txt");remove("test_a_19.txt");remove("test_a_20.txt");
	remove("test_e_11.txt");remove("test_e_12.txt");remove("test_e_13.txt");remove("test_e_14.txt");remove("test_e_15.txt");
	remove("test_e_16.txt");remove("test_e_17.txt");remove("test_e_18.txt");remove("test_e_19.txt");remove("test_e_20.txt");
	remove("test_i_11.txt");remove("test_i_12.txt");remove("test_i_13.txt");remove("test_i_14.txt");remove("test_i_15.txt");
	remove("test_i_16.txt");remove("test_i_17.txt");remove("test_i_18.txt");remove("test_i_19.txt");remove("test_i_20.txt");
	remove("test_o_11.txt");remove("test_o_12.txt");remove("test_o_13.txt");remove("test_o_14.txt");remove("test_o_15.txt");
	remove("test_o_16.txt");remove("test_o_17.txt");remove("test_o_18.txt");remove("test_o_19.txt");remove("test_o_20.txt");
	remove("test_u_11.txt");remove("test_u_12.txt");remove("test_u_13.txt");remove("test_u_14.txt");remove("test_u_15.txt");
	remove("test_u_16.txt");remove("test_u_17.txt");remove("test_u_18.txt");remove("test_u_19.txt");remove("test_u_20.txt");
	
}
