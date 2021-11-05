// atul2.cpp : Defines the entry point for the console application.
//

// Assignment_1.cpp : Defines the entry point for the console application.
//
#include "StdAfx.h"
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

static double norm_data[40000];						// This stores the normalised data after preprocess
static double word[20000] ;                         //This storeds the clipped word in an array.

int skip_line(FILE *fp) //  function to skip lines, i.e move cursor to next line !
{
    int c;

    while (c = fgetc(fp), c != '\n' && c != EOF);

    return c;
}  


int preprocess(FILE* fp_raw ,FILE* fp_dc_check, double *norm_data  ){
	
	int frame_size = 100 ;
	int Xi=0 ;
	int peak_Xi=0 ;
	long total_datapoints =0  ; 
	char line_buffer[10] ; int new_peak = 5000 ; // new_peak will be the peak of max amplitude signal after normalization.

	int line_skips = 5*frame_size; 
	while(line_skips--)skip_line(fp_raw);             // skipping some initial frames ! 5 frames here. To get rid of initial noise.

	line_skips = 5*frame_size;						  // same for dc check file.
	while(line_skips--)skip_line(fp_dc_check);  

	// DC check
	double dc_bias =0.0 ;                      
	while(!feof(fp_dc_check)){                      // Summing over all datapoints then averaging to get dc_shift if its not zero.
		fgets(line_buffer, 10, fp_dc_check);
			Xi = atoi(line_buffer);              //Converting string got from file to integer.
			dc_bias += Xi;   
			total_datapoints++;              
	}
	dc_bias = dc_bias/total_datapoints;
	printf("DC Bias we got from Silence data = %lf\n\n" , dc_bias);

	// Normalization 
	total_datapoints = 0;                   
	while(!feof(fp_raw)){ 
		fgets(line_buffer, 10, fp_raw);    // Here, I found the max amplitude signal from the data.
			Xi = atoi(line_buffer);
			if(abs(Xi) > peak_Xi ){
				peak_Xi = abs(Xi);
			} 
			total_datapoints++;    
	 }

	fseek(fp_raw , 0 , SEEK_SET);   // Setting the file ptr back to 0 and skipping initial frames again for further process.
	line_skips = 5*frame_size;
	while(line_skips--)skip_line(fp_raw); 

	double norm_mult = new_peak*1.0 / peak_Xi ;  // This gives us the normalization factor. 
	int i =0; double xi = 0.0, norm_xi = 0.0;

	while(!feof(fp_raw)){                        // I am transforming the raw data here by subtracting from each point the dc_bias , then multiplying them with 
		fgets(line_buffer, 10, fp_raw);			 // the normalization factor.
		xi = atoi(line_buffer);
		norm_xi = ((xi-dc_bias) * norm_mult );      
		if(norm_xi != 0)                                
			norm_data[i++] = norm_xi;        // Storing the normalised data in array "norm_data".
		}
	printf("This is how one Normalised datapoint look after Normalization: %lf\n\n" ,norm_data[50] );
	return i;								// returns the number of data points present in normalized data !
}

int ambient_energy_calc(){
	double cur_Xi=0.0 , prev_Xi=0.0;                 // cur_Xi is the current data point , prev_Xi is prev data point.
	int sample_num =0; 
	int ambient_energy = 0 ,avg_ambient_energy=0 , ambient_zcr =0, frame_size = 100;

	while(sample_num<=10*frame_size){               // This loop iterates for 10 frames of the start of the word which is silence and gets its energy and zcr.
		cur_Xi = norm_data[sample_num];
		if(sample_num>0){
			if((cur_Xi >= 0 && prev_Xi < 0) || (cur_Xi < 0 && prev_Xi >= 0) )ambient_zcr++;  //main logic to get zcr
			ambient_energy += cur_Xi*cur_Xi;	                    // storing sum of square of Xi , will get average after frame ends. 
		}
		prev_Xi = cur_Xi;										// this sets cur Xi as prev for next iteration.
		if(sample_num%101 ==0){                                 // checks end of frame , each time data crosses 100 points 
			avg_ambient_energy += ambient_energy/frame_size;     // Summing the avg ambient energy of all frames , later will divide by num of frames i.e 10. so we will get energy of silence .
			ambient_energy = 0;	                                 // After every frame ends , we set energy back to 0 for next iteration.
		}
		sample_num++;
	}
	avg_ambient_energy = avg_ambient_energy/10 ;           // We summed avg energy for 10 frames so dividing to get their avg energy.
	ambient_zcr = ambient_zcr/10;                         // same for ZCR

	printf("Average Energy of background noise = %d \n", avg_ambient_energy);
	printf("Average ZCR of background noise = %d \n\n", ambient_zcr);

	return avg_ambient_energy;
}

int get_word(int avg_ambient_energy , int total_datpoints ){
	double cur_Xi=0.0 , prev_Xi=0.0; int threshold_energy =0; 
	int sample_num = 1; 
	int frame_size = 100 , cur_avg_energy =0 ;
	double energy = 0.0; bool flag = false;
	int frame = 0;                                     
	int word_start_frame , word_end_frame;
	  
	while(sample_num < total_datpoints){                                // This loops the normalised data to clip out the word.
		cur_Xi =  norm_data[sample_num];

		energy += cur_Xi*cur_Xi;                                      // same process as for ambient energy
		
		if(sample_num%frame_size == 0){                              // check end of frame.
			cur_avg_energy = energy/frame_size;                    // dividing by num of data i.e 100.
			if(cur_avg_energy > 0){                               // we are checking only if avg energy > 0
			frame++;
			if(flag == false && cur_avg_energy > 2*avg_ambient_energy){  // main logic- I used flag to tell when word has started , so other condition can check for 
				word_start_frame = frame;									// end of word . I have taken threshold multiplier as 2x i.e 200% higher than ambient noise.
				printf("starting frame of word - %d\n" ,  word_start_frame); // If the cur frame's energy gets 2x higher than ambient noise , we say word started. and print the starting frame.
				threshold_energy = cur_avg_energy ;							// I set this frame's energy as threshold to check for end of word. 
				flag = true;
			}
			else if(flag == true && cur_avg_energy < threshold_energy ){    // This executes once word start has started to get end of word.
				word_end_frame = frame;										// If any frame's energy gets lower than threshold energy , we say word ended.
				printf("ending frame of word - %d\n\n" ,  word_end_frame);  // And print the end frame.
				break;
			}
		   }
		 energy = 0;                                    // After every frame ends , we set energy back to 0 for next iteration.
		}
		sample_num++;
	}
	word_start_frame = word_start_frame-1;           // Setting a buffer of 1 frame prior to word start and 3 frame after ending of word.
	word_end_frame = word_end_frame+3 ;

	int start_Xi = word_start_frame*frame_size;    // Multuplying Start and end frame with frame size to get data point marker.
	int end_Xi = word_end_frame*frame_size;       // Using this marker , we store the word in an array.
	
	 
	int i = 0;				                     // On my systm ,for some reason the file was not writing anything , I tried alot of things, still nothing worked.

	for(int xi = start_Xi ;  xi <= end_Xi ; xi++){  // Storing the word datapoints in array word[] defined statically .
		word[i] = norm_data[xi];
		i++;
	}
	int word_len = i ;        // Length of word we clipped i.e num of datapoints in word.

	return word_len ;       // returns total num of datapoints in word
}

int main()
{

	//1. Reading the Files and Preprocessing it.
	FILE* fp_raw ; FILE* fp_dc_check;   

	fopen_s(&fp_raw, "yes_raw.txt", "r") ;			// We are reading the raw file of word YES and analysing if our algorithm is working fine.
	//fopen_s(&fp_raw, "no_raw.txt", "r") ;		    // To check for word "NO", uncomment this line and comment the above line.
		
	fopen_s(&fp_dc_check, "DCshift.txt", "r");		// reading the silent file for dc shift.
		
	if (fp_raw == NULL) {						 // checking if file has opened successfully r not.
      fprintf(stderr, "error opening %s: %s", "word.txt", strerror(errno));
      return 0;
	}

	int total_datpoints = preprocess(fp_raw , fp_dc_check, norm_data) ; // See the preprocess function for this part.
	fclose(fp_raw);														// closing the files.
	fclose(fp_dc_check);

	//---------------------------------------------------------------------------------------------------------------------------------//
	//2. Ambient noise calculation

	int avg_ambient_energy = ambient_energy_calc();  // refer function ambient_energy_calc

	//-----------------------------------------------------------------------------------------------------------------------------//
	//3. Extracting word based on ambient noise and some threshold multiplier.
	int word_samples = get_word( avg_ambient_energy , total_datpoints );  // refer function get_word
	// word_samples here is total datapoints in word.

	////-------------------------------------------------------------------------------------------------------------------------------------------//
	//4. getting Energy and ZCR array of the clipped word.

	int energy_array[150];
	int zcr_array[150];

	int sample_num =1 , energy = 0, frame =0, frame_size = 100;
	double cur_Xi = 0.0 , prev_Xi = 0.0;
	int ZCR =0  ; int avg_energy=0;
	
	for(int k =0 ; k < word_samples ; k++){                   // Now we loop in the word to get energy and zcr of the word.
		cur_Xi = word[k];
		energy += cur_Xi*cur_Xi;                        // same method as before.
		 
		if(sample_num>1){
			if((cur_Xi >= 0 && prev_Xi < 0) || (cur_Xi < 0 && prev_Xi >= 0) )ZCR++;		
			}
		prev_Xi = cur_Xi;                               // this sets cur Xi as prev for next iteration.

		if(sample_num%frame_size ==0 ){                // stroing zcr and energy of each frame in arrays. 
			avg_energy = energy/frame_size;           // dividing by num of data i.e 100.
			zcr_array[frame] = ZCR;
			energy_array[frame] = avg_energy;       // frame is the frame num. After the loop , it gives no. of frames in word.
			frame++;
			ZCR =0 ; 
			energy = 0;                          // After every frame ends , we set energy,zcr back to 0 for next iteration.
		}
		sample_num++;                         // incrementing count of datapoints.
	}

	////-------------------------------------------------------------------------------------------------------//
	//5. Analysing the last part of word to make a decision.

	int total_word_len = frame;                  // total word length i.e no of frames in the word. Got from frame count of prev process.
	int last_part_start = 0.7*total_word_len ;  // starting of last part of word is from 70% to end.
	int last_part_len = 0.3*total_word_len;    // length of last 30% of word.
	printf("Length of total word - %d frames \n" , total_word_len);
	printf("Length of last 30%% of word - %d frames \n\n" , last_part_len);

	int avg_zcr = 0;
	int fricative_count = 0;   
	int vowel_count = 0; 

	for(int j = last_part_start ; j < total_word_len ; j++){     // Here , I am looping the last part of word's ZCR array to decide if word is Yes or No.
		avg_zcr += zcr_array[j] ;								// Averaging over all ZCR values.
		if(zcr_array[j] > 40)fricative_count++;				   // Checking if given frame is of fricative or vowel , will be used to get accuracy of model.		
		else if(zcr_array[j] < 25)vowel_count++;
	}

	avg_zcr /= last_part_len;
	printf("Average ZCR of last 30%%  of word - %d\n\n" , avg_zcr);  // Prining the avg ZCR of last part 

	double fricative_acc = fricative_count*100.0/last_part_len ;   // Getting percentage of fricative sound present in last part of word
	double vowel_acc = vowel_count*100.0/last_part_len ;		     // Getting percentage of vowel sound present in last part of word

	if(avg_zcr > 40)printf("THE WORD IS YES WITH %lf PERCENT ACCURACY\n" , fricative_acc);  // Finally printing what word it is based on ZCR value and its Accuracy
	else printf("THE WORD IS NO WITH %lf PERCENT ACCURACY\n " , vowel_acc);				//  based on count of the respective sounds.

	return 0 ;
}

// END OF ASSIGNMENT. THANK YOU !
