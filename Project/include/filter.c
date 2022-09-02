#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "audio.h"

#include <stdio.h>
#include <unistd.h>



#define SIZE 1024

void volumeEdit(char * buffer, int sample_size, double volumeValue){
	
	double temp;
	int lowerBound;
	int upperBound;
	

	if(sample_size == 16){
		if(volumeValue < 0.1)
			volumeValue = 0.1;
		else if(volumeValue >  20)
			volumeValue = 20;
			
		int16_t * t = (int16_t *) buffer;
		lowerBound = -32768;
		upperBound = 32767;
		
		
		for(int i=0; i<SIZE; i++){
			temp = t[i] * volumeValue;
			if(temp < lowerBound)
				t[i] = lowerBound;
			else if(temp > upperBound)
				t[i] = upperBound;
			else
				t[i] = temp;
		}
		
	}
	

	else if(sample_size == 8){
		if(volumeValue < 0.3)
			volumeValue = 0.3;
		else if(volumeValue >  1.8)
			volumeValue = 1.8;
			
		uint8_t * t = (uint8_t *) buffer;
		lowerBound = 0;
		upperBound = 255;
		
		
		for(int i=0; i<SIZE; i++){
			temp = t[i] * volumeValue;
			if(temp < lowerBound)
				t[i] = lowerBound;
			else if(temp > upperBound)
				t[i] = upperBound;
			else
				t[i] = temp;
		}
	
	
	}
}

void stereoToMono(char * buffer, int sample_size){

	int moyenne;

	if(sample_size == 8){
		uint8_t * t = (uint8_t *) buffer;
		for(int i=0; i<SIZE-1; i+=2){
			moyenne = (t[i] + t[i+1]) /2;
			t[i/2] = moyenne;
		}

	}
	
	else if (sample_size == 16){
		int16_t * t = (int16_t *) buffer;
		for(int i=0; i<SIZE-1; i+=2){
			moyenne = (t[i] + t[i+1]) /2;
			t[i/2] = moyenne;
		}
	
	}

}

void makeEcho(char * buffer, char * previousBuffer, int sample_size){

	volumeEdit(previousBuffer, sample_size, 0.5);
	
	int temp;
	int lowerBound;
	int upperBound;
	
	if(sample_size == 16){
	
		int16_t * b = (int16_t *) buffer;
		int16_t * pb = (int16_t *) previousBuffer;
		
		lowerBound = -32768;
		upperBound = 32767;
	
		for(int i=0; i<SIZE; i++){
			temp = b[i] + pb[i];
			if(temp > upperBound)
				b[i] = upperBound;
			else if (temp < lowerBound)
				b[i] = lowerBound;
			else
				b[i] = temp;
		}
		
	}
	
	if(sample_size == 8){
	
		uint8_t * b = (uint8_t *) buffer;
		uint8_t * pb = (uint8_t *) previousBuffer;
		
		lowerBound = 0;
		upperBound = 255;
	
		for(int i=0; i<SIZE; i++){
			temp = b[i] + pb[i];
			if(temp > upperBound)
				b[i] = upperBound;
			else if (temp < lowerBound)
				b[i] = lowerBound;
			else
				b[i] = temp;
		}
		
	}
	

}
