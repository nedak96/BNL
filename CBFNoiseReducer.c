#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cbf.h"
#include "cbf_string.h"
//Array for box pixels
int *nums;
//Array for all pixels of original image
int *arr;
//Counter for number of pixels in box
int n;
//Function used to insert pixel into sorted array
int insert(int num){
	int i,j;
	for (i = 0; i <= n; i++){
		if (i == n){
			nums[n] = num;
			n += 1;
			return i;
		}
		if (num < nums[i]){
			for(j = n; j > i; j--) nums[j] = nums[j-1];
			nums[i] = num;
			n += 1;
			return i;
		}
	}
	return -1;
}
//Function used to remove pixel from sorted array
int delete(int num){
	int i,j;
	for(i=0; i<n; i++){
		if(num == nums[i]){
			for(j=i; j<n;j++) nums[j] = nums[j+1];
			n -= 1;
			return i;
		}
	}
	return -1;
}
//Main
int main(int argc, char **argv){
	//Create cbf handles for original image and output image
	cbf_handle handle, cbf;
	//Create info for new image
	cbf_failnez (cbf_make_handle (&cbf));
	cbf_failnez(cbf_new_datablock(cbf, "spots"));
  	cbf_failnez(cbf_new_category(cbf,"array_data"));
  	cbf_failnez(cbf_new_column(cbf,"data"));
  	cbf_failnez(cbf_new_row(cbf));
	//Open original image from command line argument
	cbf_failnez(cbf_make_handle(&handle)); 
	FILE *fp = fopen(argv[1], "rb");
	if(fp == NULL){
		printf("File does not exist\n");
		return 0;
	}
	//Declare box size and allocate memory for box in nums array
	int box = 15;
	char* outfile = "outfile.cbf";
	int u;
	for(u=1; u<argc-1; u++){
		if(!strcmp(argv[u],"-b")) box = strtol(argv[u+1], NULL, 10);
		else if(!strcmp(argv[u], "-o")) outfile = argv[u+1];
	}
	nums = malloc(sizeof(int)*(box*2+1)*(box*2+1));
	//Go to pixel data in original image
	cbf_failnez(cbf_read_file(handle, fp, MSG_DIGEST));
	cbf_failnez(cbf_find_category(handle,"array_data"));
	cbf_failnez(cbf_find_column(handle,"data"));
	cbf_failnez(cbf_rewind_row(handle));
	size_t nelem, elsize, dim1, dim2, dim3, padding, numread;
	const char *byteorder;
	unsigned int compression;
	int id, elsigned, elunsigned, minel, maxel, i=0;
	//Get information about image
	cbf_failnez(cbf_get_integerarrayparameters_wdims(handle, &compression, &id, &elsize, &elsigned, &elunsigned, &nelem, &maxel, &minel, &byteorder, &dim1, &dim2, &dim3, &padding));
	//Allocate memory for original image array
	arr = malloc(sizeof(int)*nelem);
	//Allocate memory for new image array
	int *arr2 = calloc(nelem, sizeof(int));
	//Get pixel array and put in arr
	cbf_failnez(cbf_get_integerarray(handle, NULL, arr, sizeof (int), 0, nelem, &numread));
	//x,y = counter
	int y, x, j;
	//start and end values 
	int endx = dim1;
	int endy = dim2;
	int starty = 0;
	int startx = 0;
	//sum of middle 80%, sum of middle 80%^2
	unsigned long sum = 0, smos = 0;
	n = 0;
	//Number of pixels in bottom 10%, number of pixels in the top 10%
	int left, right;
	double sd, average;
	//Find spot slices
	for(y=starty; y<endy; y++){
		left = (int)(.1*n);
		right = n-(int)(.9*n);
		int bot = endy-1;
		int top = starty;
		//If it is an even y value, traverse row left to right
		if (y%2 == 0){
			//If y is at the beginning, initialize box
			if (y == starty){
				bot = starty + box;
				for(j = starty; j<=starty+box; j++){
					for (i=startx; i <= startx+box; i++){
						if(arr[j*dim1+i] != -1){
							insert(arr[j*dim1+i]);
						}
					}
				}
				for (i=(int)(.1*n); i<(int)(.9*n); i++){
					sum += nums[i];
					smos += nums[i]*nums[i];
				}
			}
			else{
				//If the upper half of the box fits, delete uppermost row
				if(y-box-1 >= starty){
					top = y-box;
					for(i=startx; i<= startx+box; i++){
						int val = arr[(y-box-1)*dim1+i];
						if (val != -1){
							int ind = delete(val);
							if(ind < left) left -= 1;
							else if(ind >= n-right) right -= 1;
							else{
								sum -= val;
								smos -= val*val;
							}
						}
					}
				}
				//If the lower half of the box fits, insert next row below box
				if(y+box < endy){
					bot = y+box;
					for(i= startx; i <= startx+box; i++){
						int val = arr[(y+box)*dim1+i];
						if (val != -1){
							int ind = insert(val);
							if (ind < left) left += 1;
							else if(ind >= n-right) right += 1;
							else{
								sum += val;
								smos += val*val;
							}
						}
					}
				}
				//If there are too few pixels in the bottom 10%, subtract them from from the sum and sum^2
				if (left < (int)(.1*n)){
					for (i=left; i<(int)(.1*n); i++){
						sum -= nums[i];
						smos -= nums[i]*nums[i];
					}
				}
				//If there are too many pixels in the bottom 10%, add them from from the sum and sum^2
				else if (left > (int)(.1*n)){
					for(i=(int)(.1*n); i<left; i++){
						sum += nums[i];
						smos += nums[i]*nums[i];
					}
				}
				//If there are too few pixels in the top 10%, subtract them from from the sum and sum^2
				if (right < n-(int)(.9*n)){
					for (i=(int)(.9*n); i<n-right; i++){
						sum -= nums[i];
						smos -= nums[i]*nums[i];
					}
				}
				//If there are too many pixels in the top 10%, add them from from the sum and sum^2
				else if (right > n-(int)(.9*n)){
					for (i= n-right; i<(int)(.9*n); i++){
						sum += nums[i];
						smos += nums[i]*nums[i];
					}
				}
			}
			for(x=startx; x<endx; x++){
				left = (int)(.1*n);
				right = n-(int)(.9*n);
				//If x is at the beginning of the row, dont do anything
				if (x != startx){
					//If left half of box fits, delete leftmost column
					if(x-box-1 >= startx){
						for(i=top; i<=bot; i++){
							int val = arr[i*dim1+x-box-1];
							if (val != -1){
								int ind = delete(val);
								if(ind < left) left -= 1;
								else if(ind >= n-right) right -= 1;
								else{
									sum -= val;
									smos -= val*val;
								}
							}
						}
					}
					//If right half of box fits, insert next column to the right
					if(x+box < endx){
						for(i= top; i <= bot; i++){
							int val = arr[i*dim1+x+box];
							if (val != -1){
								int ind = insert(val);
								
								if (ind < left) left += 1;
								else if(ind >= n-right) right += 1;
								else{
									sum += val;
									smos += val*val;
								}
							}
						}
					}
					//If there are too few pixels in the bottom 10%, subtract them from from the sum and sum^2
					if (left < (int)(.1*n)){
						for (i=left; i<(int)(.1*n); i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					//If there are too many pixels in the bottom 10%, add them from from the sum and sum^2
					else if (left > (int)(.1*n)){
						for(i=(int)(.1*n); i<left; i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
					//If there are too few pixels in the top 10%, subtract them from from the sum and sum^2
					if (right < n-(int)(.9*n)){
						for (i=(int)(.9*n); i<n-right; i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					//If there are too many pixels in the top 10%, add them from from the sum and sum^2
					else if (right > n-(int)(.9*n)){
						for (i= n-right; i<(int)(.9*n); i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
				}
				//Calculate standard deviation and average
				sd = sqrt(fabs(((.9*n-.1*n)*smos-sum*sum)/((.9*n-.1*n)*(.9*n-.1*n-1))));
				average = sum/(.9*n-.1*n);
				//If pixel is __ standard deviations above the mean, it is considered a spot
				if(arr[y*dim1+x] > average+sd*5) arr2[y*dim1+x] = arr[y*dim1+x];
				//If pixel isn't a spot, average with average
				else arr2[y*dim1+x] = (arr[y*dim1+x]+average)/2;
			}	
		}
		//If y is an odd value, traverse row right to left
		else{
			//If upper half of box fits, delete uppermost row
			if(y-box-1 >= starty){
				top = y-box;
				for(i=endx-box-1; i< endx; i++){
					int val = arr[(y-box-1)*dim1+i];
					if (val != -1){
						int ind = delete(val);
						if(ind < left) left -= 1;
						else if(ind >= n-right) right -= 1;
						else{
							sum -= val;
							smos -= val*val;
						}
					}
				}
			}
			//If lower half of box fits, insert next row below the box
			if(y+box < endy){
				bot = y+box;
				for(i= endx-box-1; i < endx; i++){
					int val = arr[(y+box)*dim1+i];
					if (val != -1){
						int ind = insert(val);
						if (ind < left) left += 1;
						else if(ind >= n-right) right += 1;
						else{
							sum += val;
							smos += val*val;
						}
					}
				}
			}
			//If there are too few pixels in the bottom 10%, subtract them from from the sum and sum^2
			if (left < (int)(.1*n)){
				for (i=left; i<(int)(.1*n); i++){
					sum -= nums[i];
					smos -= nums[i]*nums[i];
				}
			}
			//If there are too many pixels in the bottom 10%, add them from from the sum and sum^2
			else if (left > (int)(.1*n)){
				for(i=(int)(.1*n); i<left; i++){
					sum += nums[i];
					smos += nums[i]*nums[i];
				}
			}
			//If there are too few pixels in the top 10%, subtract them from from the sum and sum^2
			if (right < n-(int)(.9*n)){
				for (i=(int)(.9*n); i<n-right; i++){
					sum -= nums[i];
					smos -= nums[i]*nums[i];
				}
			}
			//If there are too many pixels in the top 10%, add them from from the sum and sum^2
			else if (right > n-(int)(.9*n)){
				for (i= n-right; i<(int)(.9*n); i++){
					sum += nums[i];
					smos += nums[i]*nums[i];
				}
			}
			for(x=endx-1; x>=startx; x--){
				left = (int)(.1*n);
				right = n-(int)(.9*n);
				//If x is at the end of the row, do nothing
				if(x != endx-1){
					//If right half of box fits, delete rightmost column
					if(x+box+1 < endx){
						for(i=top; i<=bot; i++){
							int val = arr[i*dim1+x+box+1];
							if (val != -1){
								int ind = delete(val);
								if(ind < left) left -= 1;
								else if(ind >= n-right) right -= 1;
								else{
									sum -= val;
									smos -= val*val;
								}
							}
						}
					}
					//If left half of box fits, insert next column to the left
					if(x-box >= startx){
						for(i= top; i <= bot; i++){
							int val = arr[i*dim1+x-box];
							if (val != -1){
								int ind = insert(val);
								if (ind < left) left += 1;
								else if(ind >= n-right) right += 1;
								else{
									sum += val;
									smos += val*val;
								}
							}
						}
					}
					//If there are too few pixels in the bottom 10%, subtract them from from the sum and sum^2
					if (left < (int)(.1*n)){
						for (i=left; i<(int)(.1*n); i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					//If there are too many pixels in the bottom 10%, add them from from the sum and sum^2
					else if (left > (int)(.1*n)){
						for(i=(int)(.1*n); i<left; i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
					//If there are too few pixels in the top 10%, subtract them from from the sum and sum^2
					if (right < n-(int)(.9*n)){
						for (i=(int)(.9*n); i<n-right; i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					//If there are too many pixels in the top 10%, add them from from the sum and sum^2
					else if (right > n-(int)(.9*n)){
						for (i= n-right; i<(int)(.9*n); i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
				}
				//Calculate standard deviation and average
				sd = sqrt(fabs(((.9*n-.1*n)*smos-sum*sum)/((.9*n-.1*n)*(.9*n-.1*n-1))));
				average = sum/(.9*n-.1*n);
				//If pixel is __ standard deviations above the mean, it is considered a spot
				if(arr[y*dim1+x] > average+sd*5) arr2[y*dim1+x] = arr[y*dim1+x];
				//If pixel isn't a spot, average with average
				else arr2[y*dim1+x] = (arr[y*dim1+x]+average)/2;
			}
		}
	}
	//Write new image array to out.cbf
	cbf_failnez(cbf_set_integerarray_wdims(handle, compression, id, arr2, elsize, elsigned, nelem, byteorder, dim1, dim2, dim3, padding));
	FILE *f = fopen(outfile, "wb");
	cbf_failnez (cbf_write_file (handle, f, 4, CBF, MSG_DIGEST | MIME_HEADERS, 0));
	//Free arrays
	free(nums);
	free(arr);
	free(arr2);
	//Close files
	fclose(fp);
	fclose(f);
	return 0;
}
