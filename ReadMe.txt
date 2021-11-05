========================================================================
             ASSIGNMENT 1 :  YES/NO PREDICTION 
========================================================================

Mode for Speech processing Course in IIT Guwahati. It correctly predicts whether the said word is YES or NO.

To Execute:-
I have used only one main file for the entirety of the assignment- "main.cpp".
just run this file. Make sure all text files are in same folder as the main file.

Text Files used:-
1. yes_raw.txt - Contains the signals for word yes taken from cooledit.
2. no_raw.txt -  Contains the signals for word no taken from cooledit.
3. DCshift.txt - Contains the signals for audio taken with mic off to get the dc shi.

Functions used:-
1. skip_line() - skips a line in the file. Used to skip some initial readings of the data.
2. preprocess() - Performs dc shift fix and normalises the data. Stores all normalised points in array "norm_data[]". Returns the number of datapoints/samples present in normalised data.

4. ambient_energy_calc() - Calculates average energy of background noise and returns it.

5. get_word() - To clip datapoints of spoken word from whole data. stores them in array "word[]". Returns total num of datapoints in word.

6. main() - All above functions are called and word analysis is done.  energy and zcr of word is calculated and decision is made based on it.

Flow of the main() function :- 
1. Preprocess - Transforming raw input to processed data stored in array norm_data[].

2. Ambient noise calculation - Calculating background noise energy.

3. Extracting word based on ambient noise and some threshold multiplier(3x) - Using background noise energy, I clipped out the word datapoints. stored in array word[]

4. getting Energy and ZCR from the clipped word - Stored the energy and zcr of word in 2 arrays energy_array[] and zcr_array[]. 

5. Analysing the last part of word to make a decision - Used the zcr_array to decide if the word is yes or no.


/////////////////////////////////////////////////////////////////////////////
