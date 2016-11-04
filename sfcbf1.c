#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cbf.h"

//change first number to size of box
int nums[(15*2+1)*(15*2+1)];
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
	cbf_handle handle;
	cbf_failnez(cbf_make_handle(&handle)); 
	FILE *fp = fopen("1191_00005.cbf", "rb");
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
	int startx = 0, starty = 0, endx = dim1, endy = dim2, box = 15, y, x, i, j;
	unsigned long sum = 0, smos = 0;
	n = 0;
	FILE *f = fopen("out.pgm", "wb");
	fprintf(f, "P5\n%i %i\n255\n", dim1, dim2);
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
				float sd = sqrt(abs(((9*n/10-n/10)*smos-sum*sum)/((9*n/10-n/10)*(9*n/10-n/10-1))));
				float average = (float)sum/((9*n/10)-(n/10));
				if(arr[y*dim1+x] > average+sd*5) fputc(254, f);
				else fputc(1, f);
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
				float sd = sqrt(abs(((9*n/10-n/10)*smos-sum*sum)/((9*n/10-n/10)*(9*n/10-n/10-1))));
				float average = (float)sum/(9*n/10-n/10);
				if(arr[y*dim1+x] > average+sd*5) fputc(254, f);
				else fputc(1, f);
			}
		}
	}
	fclose(f);
	return 0;
}
