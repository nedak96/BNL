#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cbf.h"

//change first number to size of box
int *nums;
int *arr;
int n;

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

int main(int argc, char **argv){
	if(argc < 3){
		printf("Format: input.cbf output.cbf\n");
		return 0;
	}
	cbf_handle handle, cbf;
	cbf_failnez(cbf_make_handle(&handle)); 
	FILE *fp = fopen(argv[1], "rb");
	if(fp == NULL){
		printf("File does not exist\n");
		return 0;
	}
	int box;
	if(argc == 4){ 
		box = strtol(argv[3], NULL, 10);
	}
	else box = 15;
	nums = malloc(sizeof(int)*(box*2+1)*(box*2+1));
	cbf_failnez(cbf_read_file(handle, fp, MSG_DIGEST));
	cbf_failnez(cbf_find_category(handle,"array_data"));
	cbf_failnez(cbf_find_column(handle,"data"));
	cbf_failnez(cbf_rewind_row(handle));
	size_t nelem, elsize, dim1, dim2, dim3, padding, numread;
	const char *byteorder;
	unsigned int compression;
	int id, elsigned, elunsigned, minel, maxel;
	cbf_failnez(cbf_get_integerarrayparameters_wdims(handle, &compression, &id, &elsize, &elsigned, &elunsigned, &nelem, &maxel, &minel, &byteorder, &dim1, &dim2, &dim3, &padding));
	arr = malloc(sizeof(int)*nelem);
	cbf_failnez(cbf_get_integerarray(handle, NULL, arr, sizeof (int), 0, nelem, &numread));
	int startx = 0, starty = 0, endx = dim1, endy = dim2, y, x, i, j;
	unsigned long sum = 0, smos = 0;
	n = 0;
	cbf_failnez (cbf_make_handle (&cbf));
	cbf_failnez(cbf_new_datablock(cbf, "spots_1191_00005"));
  	cbf_failnez(cbf_new_category(cbf,"array_data"));
  	cbf_failnez(cbf_new_column(cbf,"data"));
  	cbf_failnez(cbf_new_row(cbf));
	int *arr2 = malloc(sizeof(int)*nelem);
	for(y=starty; y<endy; y++){
		int left = n/10;
		int right = n-9*n/10;
		int bot = endy-1;
		int top = starty;
		if (y%2 == 0){
			if (y == starty){
				bot = starty + box;
				for(j = starty; j<=starty+box; j++){
					for (i=startx; i <= startx+box; i++){
						if(arr[j*dim1+i] != -1){
							insert(arr[j*dim1+i]);
						}
					}
				}
				for (i=n/10; i<9*n/10; i++){
					sum += nums[i];
					smos += nums[i]*nums[i];
				}
			}
			else{
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
				if (left < n/10){
					for (i=left; i<n/10; i++){
						sum -= nums[i];
						smos -= nums[i]*nums[i];
					}
				}
				else if (left > n/10){
					for(i=n/10; i<left; i++){
						sum += nums[i];
						smos += nums[i]*nums[i];
					}
				}
				if (right < n-9*n/10){
					for (i=9*n/10; i<n-right; i++){
						sum -= nums[i];
						smos -= nums[i]*nums[i];
					}
				}
				else if (right > n-9*n/10){
					for (i= n-right; i<9*n/10; i++){
						sum += nums[i];
						smos += nums[i]*nums[i];
					}
				}
			}
			for(x=startx; x<endx; x++){
				left = n/10;
				right = n-9*n/10;
				if (x != startx){
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
					if (left < n/10){
						for (i=left; i<n/10; i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					else if (left > n/10){
						for(i=n/10; i<left; i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
					if (right < n-9*n/10){
						for (i=9*n/10; i<n-right; i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					else if (right > n-9*n/10){
						for (i= n-right; i<9*n/10; i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
				}
				float sd = sqrt(fabs(((.9*n-.1*n)*smos-sum*sum)/((.9*n-.1*n)*(.9*n-.1*n-1))));
				float average = sum/(.9*n-.1*n);
				if(arr[y*dim1+x] > average+sd*5) arr2[y*dim1+x] = 1;
				else arr2[y*dim1+x] = 0;
			}	
		}
		else{
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
			if (left < n/10){
				for (i=left; i<n/10; i++){
					sum -= nums[i];
					smos -= nums[i]*nums[i];
				}
			}
			else if (left > n/10){
				for(i=n/10; i<left; i++){
					sum += nums[i];
					smos += nums[i]*nums[i];
				}
			}
			if (right < n-9*n/10){
				for (i=9*n/10; i<n-right; i++){
					sum -= nums[i];
					smos -= nums[i]*nums[i];
				}
			}
			else if (right > n-9*n/10){
				for (i= n-right; i<9*n/10; i++){
					sum += nums[i];
					smos += nums[i]*nums[i];
				}
			}
			for(x=endx-1; x>=startx; x--){
				left = n/10;
				right = n-9*n/10;
				if(x != endx-1){
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
					if (left < n/10){
						for (i=left; i<n/10; i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					else if (left > n/10){
						for(i=n/10; i<left; i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
					if (right < n-9*n/10){
						for (i=9*n/10; i<n-right; i++){
							sum -= nums[i];
							smos -= nums[i]*nums[i];
						}
					}
					else if (right > n-9*n/10){
						for (i= n-right; i<9*n/10; i++){
							sum += nums[i];
							smos += nums[i]*nums[i];
						}
					}
				}
				float sd = sqrt(fabs(((.9*n-.1*n)*smos-sum*sum)/((.9*n-.1*n)*(.9*n-.1*n-1))));
				float average = sum/(.9*n-.1*n);
				if(arr[y*dim1+x] > average+sd*5) arr2[y*dim1+x] = 1;
				else arr2[y*dim1+x] = 0;
			}
		}
	}
	cbf_failnez(cbf_set_integerarray_wdims(cbf, 96, 0, arr2, 4, 1, nelem, "little_endian", dim1, dim2, dim3, 0));
	FILE *f = fopen(argv[2], "wb");
	cbf_failnez (cbf_write_file (cbf, f, 1, CBF, MSG_DIGEST | MIME_HEADERS, 0));
	return 0;
}
