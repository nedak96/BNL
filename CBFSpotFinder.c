#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cbf.h"
#include "cbf_string.h"
#include <CVector.h>
#include <rhrand.h>
#include <CNearTree.h>
#include <time.h>
//Array for box pixels
int *nums;
//Array for all pixels of original image
int *arr;
//Counter for number of pixels in box
int n;
//Nodes for Queue
struct Node {
	double data[3];
	struct Node* next;
};
struct Node* front = NULL;
struct Node* rear = NULL;
//Enqueue function
void Enqueue(double x[3]) {
	struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
	temp->data[0] = x[0];
	temp->data[1] = x[1]; 
	temp->data[2] = x[2]; 
	temp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = temp;
		return;
	}
	rear->next = temp;
	rear = temp;
}
//Dequeue function
void Dequeue(double out[3]){
	struct Node* temp = front;
	if(front == NULL) return;
	out[0] = temp->data[0];
	out[1] = temp->data[1]; 
	out[2] = temp->data[2]; 
	if(front == rear) front = rear = NULL;
	else front = front->next;
	free(temp);
}
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
	//Get to the header contents of orginal image
	cbf_failnez(cbf_read_file(handle, fp, MSG_DIGEST));
	cbf_failnez(cbf_find_category(handle,"array_data"));
	cbf_failnez(cbf_find_column(handle,"header_contents"));
	cbf_failnez(cbf_rewind_row(handle));
	//temporary const char* to get header contents and translate to mutable char*
	const char * info1;
	cbf_failnez(cbf_get_value(handle, &info1));
	//mutable char* with header contents
	char *info = strdup(info1);
	//tokenize the header to get information
	char *token = strtok(info, " ");
	double lambda, dist, raster, bx, by, start, scale, end, rtemp, stemp, xtemp, ytemp, ztemp, avga, sina, cosa, ddist, doubleVar;
	while(token != NULL){
		//lambda = wavelength
		if(!cbf_cistrcmp(token, "Wavelength")){
			token = strtok(NULL, " ");
			lambda = strtod(token, NULL);
		}
		//Dist = detector distance, ddist = detector distance^2
		else if(!cbf_cistrcmp(token, "Detector_distance")){
			token = strtok(NULL, " ");
			dist = strtod(token, NULL);
			dist = dist*1000;
			ddist = dist*dist;
		}
		//raster = size of x value pixel, scale = size of x pixel/size of y pixel
		else if(!cbf_cistrcmp(token, "Pixel_size")){
			token = strtok(NULL, " ");
			int exp = strtod(token+(strlen(token)-2), NULL);
			printf("Exponent: %d\n", exp);
			token[strlen(token)-3]=0;
			raster = strtod(token, NULL);
			raster = pow(10, exp+3)*raster;
			printf("Raster: %g\n", raster);
			token = strtok(NULL, " ");
			token = strtok(NULL, " ");
			token = strtok(NULL, " ");
			exp = strtod(token+(strlen(token)-2), NULL);
			printf("Exponent: %d\n", exp);
			token[strlen(token)-3]=0;
			scale = strtod(token, NULL);
			scale = pow(10, exp+3)*scale;
			printf("Raster: %g\n", scale);
			scale = raster/scale;
			printf("Scale (should be 1): %g\n", scale);
		}
		//bx = beamstop in x, by = beamstop in y
		else if(!cbf_cistrcmp(token, "Beam_xy")){
			token = strtok(NULL, " ");
			token++;
			token[strlen(token)-1] = 0;
			bx = strtod(token, NULL);
			printf("Bx: %g\n", bx);
			token = strtok(NULL, " ");
			token[strlen(token)-1] = 0;
			by = strtod(token, NULL);
			printf("By: %g\n", by);
		}
		//start = start angle
		else if(!cbf_cistrcmp(token, "Start_angle")){
			token = strtok(NULL, " ");
			start = strtod(token, NULL);
		}
		//end = end angle
		else if(!cbf_cistrcmp(token, "Angle_increment")){
			token = strtok(NULL, " ");
			end = strtod(token, NULL);
			end += start;
		}
		token = strtok(NULL, " ");
	}
	//average angle
	avga = -(start+end)/2;
	avga = avga/180.*M_PI;
	printf("%g %g %g\n", start, end, avga);
	//sine of average angle
	sina = sin(avga);
	//cosine of average angle
	cosa = cos(avga);
	printf("%g %g\n", sina, cosa);
	//Go to pixel data in original image
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
	//x,y = counter, cont = keeps track if last pixel was a spot or not, spotind = current number of spots
	int y, x, j, cont=0, spotind=0;
	//start and end values 
	int endx = dim1;
	int endy = dim2;
	int starty = 0;
	int startx = 0;
	//sum of middle 80%, sum of middle 80%^2
	unsigned long sum = 0, smos = 0;
	n = 0;
	//Allocate an array, containing an array of 5 doubles:
	//	start x value
	//	end x value
	//	y value
	//	average
	//	flag to check if it was used
	double **spots = malloc(sizeof(double*)*nelem/2);
	for(i=0; i<nelem/2; i++)spots[i]=malloc(5*sizeof(double));
	//Number of pixels in bottom 10%, number of pixels in the top 10%
	int left, right;
	double sd, average;
	//Find spot slices
	for(y=starty; y<endy; y++){
		left = (int)(.1*n);
		right = n-(int)(.9*n);
		int bot = endy-1;
		int top = starty;
		//If the y value was continued, reset it and increment number of spots
		if(cont){
			cont = 0;
			spotind++;
		}
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
				if(arr[y*dim1+x] > average+sd*5){
					//If continued, increment end x value and add average 
					if(cont){
						spots[spotind][1]++;
						spots[spotind][3] += average;
					}
					//If not continued, make new spot
					else{
						spots[spotind][0]=x;
						spots[spotind][1]=x;
						spots[spotind][2]=y;
						spots[spotind][3]=average;
						spots[spotind][4]=0;
						cont=1;
					}
					//Make pixel in new image a spot
					arr2[y*dim1+x] = 100;
				}
				//If pixel isn't a spot and continued, reset cont flag and increment number of spots
				else if(cont){
					cont = 0;
					spotind++;
				}
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
				if(arr[y*dim1+x] > average+sd*5){
					//If continued, decrement start x value and add average 
					if(cont){
						spots[spotind][0]--;
						spots[spotind][3] += average;
					}
					//If not continued, make new spot
					else{
						spots[spotind][0]=x;
						spots[spotind][1]=x;
						spots[spotind][2]=y;
						spots[spotind][3]=average;
						spots[spotind][4]=0;
						cont=1;
					}
					//Make pixel in new image a spot
					arr2[y*dim1+x] = 100;
				}
				//If pixel isn't a spot and continued, reset cont flag and increment number of spots
				else if(cont){
					cont = 0;
					spotind++;
				}
			}
		}
	}
	//If continued, increment number of spots
	if(cont)spotind++;
	//Sum1 = sum of the averages for overall average, xsum = sum of averages for weighted x average, ysum = sum of averages for weighted y average
	double xsum, ysum, sum1;
	//Create a NearTree with 3d coordinates
	CNearTreeHandle tree;
	CNearTreeCreate(&tree,3,CNEARTREE_TYPE_DOUBLE);
	double v[3];
	//Number of pixels in the spot
	int k;
	//Put together the spot slices
	for(i=0; i<spotind; i++){
		//If spot hasn't been used
		if(!spots[i][4]){
			//Set used flag
			spots[i][4] = 1;
			sum1 = spots[i][3];
			k = spots[i][1]-spots[i][0]+1;
			//x is used to hold the index of the last spot slice used
			x = i;
			//y is used to hold the y value of the last spot slice used
			y = spots[i][2];
			for(j=i+1; j<spotind && spots[j][2] <= y+1; j++){
				//if spot slice isn't used, and overlaps with last spot slice that was used, add it to the spot
				if(!spots[j][4] && !((spots[j][0]<spots[x][0] && spots[j][1]<spots[x][0]) || (spots[j][0]>spots[x][1] && spots[j][1]>spots[x][1]))){
					sum1 += spots[j][3];
					k += (spots[j][1]-spots[j][0]+1);
					x = j;
					y++;
				}
			}
			//if spot contains more than 3 pixels
			if(k>3){
				//calculate overall average
				average = sum1/k;
				xsum = 0;
				ysum = 0;
				x = i;
				y = spots[i][2];
				sum1 = 0;
				//for each pixel in the spot slice, add (the difference of the pixel value and the overall average)*x/y/1 value to the xsum/ysum/sum1
				for(j=spots[i][0]; j<=spots[i][1]; j++){
					xsum += j*(arr[(int)spots[i][2]*dim1+j]-average);
					ysum += spots[i][2]*(arr[(int)spots[i][2]*dim1+j]-average);
					sum1 += (arr[(int)spots[i][2]*dim1+j]-average);
					printf("x value: %d y value: %d pixel value: %d average: %g\n", j, (int)spots[i][2], arr[(int)spots[i][2]*dim1+j], average);
					printf("xsum: %g ysum: %g sum1: %g\n", xsum, ysum, sum1);
				}
				for(j=i+1; j<spotind && spots[j][2] <= y+1; j++){
					//if spot slice isn't used, and overlaps with last spot slice that was used
					if(!spots[j][4] && !((spots[j][0]<spots[x][0] && spots[j][1]<spots[x][0]) || (spots[j][0]>spots[x][1] && spots[j][1]>spots[x][1]))){
						//for each pixel in the spot slice, add (the difference of the pixel value and the overall average)*x/y/1 value to the xsum/ysum/sum1
						for(k=spots[j][0]; k<=spots[j][1]; k++){
							xsum += k*(arr[(int)spots[j][2]*dim1+k]-average);
							ysum += spots[j][2]*(arr[(int)spots[j][2]*dim1+k]-average);
							sum1 += (arr[(int)spots[j][2]*dim1+k]-average);
							printf("x value: %d y value: %d pixel value: %d average: %g\n", k, (int)spots[i][2], arr[(int)spots[i][2]*dim1+j], average);
							printf("xsum: %g ysum: %g sum1: %g\n", xsum, ysum, sum1);
						}
						y++;
						x = j;
						//Set used flag
						spots[j][4] = 1;
					}
				}
				//calculate the x and y values of the center of the spot
				xsum /= sum1;
				ysum /= sum1;
				printf("%g %g\n", xsum, ysum);
				//Make center pixel of spot darker
				arr2[(int)round(ysum)*dim1+(int)round(xsum)] = 110;
				//calculate 3d coordinates of 2d spot center
				rtemp = (xsum-bx)*raster;
				stemp = (ysum-by)*raster*scale;
				printf("rtemp: %g stemp: %g\n", ysum, by, stemp);
				doubleVar = 1/(lambda*sqrt(rtemp*rtemp+stemp*stemp+ddist));
				printf("DV: %g\n", doubleVar);
				xtemp = rtemp/doubleVar;
				ytemp = stemp/doubleVar;
				ztemp = doubleVar*dist-(1/lambda);
				v[0] = xtemp*cosa+ztemp*sina;
				v[1] = ytemp;
				v[2] = xtemp*-1*sina+ztemp*cosa;
				printf("%g %g %g\n", v[0], v[1], v[2]);
				//Add spot to the near tree
				CNearTreeInsert(tree,&v[0],NULL);
			}
		}
	}
	//Write new image array to out.cbf
	cbf_failnez(cbf_set_integerarray_wdims(cbf, 96, 0, arr2, 4, 1, (endx-startx)*(endy-starty), "little_endian", endx-startx, endy-starty, dim3, 0));
	FILE *f = fopen(outfile, "wb");
	cbf_failnez (cbf_write_file (cbf, f, 4, CBF, MSG_DIGEST | MIME_HEADERS, 0));
	//Get random spot from the tree
	i = rand()%CNearTreeSize(tree);
	//Used as a return pointer for NearTree functions
	void *foundpoint = CNearTreeCoordAt(tree, i);
	//Used to translate foundpoint to a double pointer
	double *vector = (double *) foundpoint, *vector2;
	//Used as a search vector in NearTree functions
	double vSearch[3] = {vector[0], vector[1], vector[2]};
	//Temporary array for vector
	double tarr[3];
	CVectorHandle vReturn;
	CVectorCreate(&vReturn,sizeof(void *),CNearTreeSize(tree));
	//Get 100 nearest spots to the random spot and put in vReturn
	CNearTreeFindKNearest(tree, 100, 3000, vReturn, NULL, vSearch, 1);
	//Create another NearTree of 3d coordinates for vectors
	CNearTreeHandle tree2;
	CNearTreeCreate(&tree2,3,CNEARTREE_TYPE_DOUBLE);
	//Find vectors between all spots in vReturn and put in tree2
	for(i=0; i<vReturn->size-1; i++){
		CVectorGetElement(vReturn,&foundpoint,i);
		vector = (double *) foundpoint;
		for(j=i+1; j<vReturn->size; j++){
			CVectorGetElement(vReturn,&foundpoint,j);
			vector2 = (double *) foundpoint;
			v[0] = vector[0]-vector2[0];
			v[1] = vector[1]-vector2[1];
			v[2] = vector[2]-vector2[2];
			//Translate all vectors to the +x octants
			if(v[0]<0){
				v[0] = -v[0];
				v[1] = -v[1];
				v[2] = -v[2];
			}
			CNearTreeInsert(tree2,&v[0],NULL);
		}
	}
	vSearch[0] = 0;
	vSearch[1] = 0;
	vSearch[2] = 0;
	//Make tree contain all vectors from tree2 that are between 1.5 and 300 in length
	CNearTreeFindTreeInAnnulus(tree2, 1.5, 300, tree, vSearch,1);
	CNearTreeClear(tree2);
	CNearTreeHandle temptree;
	CNearTreeCreate(&temptree,3,CNEARTREE_TYPE_DOUBLE);
	//While tree is not empty
	while(CNearTreeSize(tree) > 0){
		//Get first vector in tree
		foundpoint = CNearTreeCoordAt(tree, 0);
		vector = (double*)foundpoint;
		v[0] = vector[0];
		v[1] = vector[1];
		v[2] = vector[2];
		//Enqueue vector
		Enqueue(v);
		i = 1;
		//While queue is not empty
		while(front != NULL){
			//Dequeue vector
			Dequeue(vSearch);
			//Put all vectors within 2% of the vecotr in vReturn
			CNearTreeFindInSphere(tree, .02*sqrt(vSearch[0]*vSearch[0]+vSearch[1]*vSearch[1]+vSearch[2]*vSearch[2]), vReturn, NULL, vSearch, 1);
			//Sum vectors in vReturn
			for(j=1; j<vReturn->size; j++){
				CVectorGetElement(vReturn, &foundpoint, j);
				vector = (double *) foundpoint;
				tarr[0] = vector[0];
				tarr[1] = vector[1];
				tarr[2] = vector[2];
				v[0] += vector[0];
				v[1] += vector[1];
				v[2] += vector[2];
				Enqueue(tarr);
				i++;
			}
			//Make temptree contain all vectors outside 2% of the original vector
			CNearTreeFindTreeOutSphere(tree, .02*sqrt(vSearch[0]*vSearch[0]+vSearch[1]*vSearch[1]+vSearch[2]*vSearch[2]), temptree, vSearch, 1);
			//Copy contents of temptree to tree
			CNearTreeFindTreeOutSphere(temptree, 0, tree, vSearch, 1);
		}
		//Find averages of the vector
		v[0] /= i;
		v[1] /= i;
		v[2] /= i;
		//Insert averaged vectors to tree2
		CNearTreeInsert(tree2,&v[0],NULL);
	}
	vSearch[0] = 0;
	vSearch[1] = 0;
	vSearch[2] = 0;
	//Find 200 smallest vectors in tree2 and put in vReturn
	CNearTreeFindKNearest(tree2, 200, 300, vReturn, NULL, vSearch, 1);
	//Array to hold 3d vector coordinates and lengths
	double smallestVectors[3][4];
	i = 1;
	j = 1;
	double a1, a2, a3;
	smallestVectors[0][0] = 0;
	smallestVectors[0][1] = 0;
	smallestVectors[0][2] = 0;
	smallestVectors[0][3] = 0;
	smallestVectors[1][0] = 0;
	smallestVectors[1][1] = 0;
	smallestVectors[1][2] = 0;
	smallestVectors[1][3] = 0;
	smallestVectors[2][0] = 0;
	smallestVectors[2][1] = 0;
	smallestVectors[2][2] = 0;
	smallestVectors[2][3] = 0;
	//Try to find 3 ~perpendicular vectors in vReturn
	for(y = 0; y<vReturn->size-2; y++){
		CVectorGetElement(vReturn, &foundpoint, y);
		vector = (double *) foundpoint;
		smallestVectors[0][0] = vector[0];
		smallestVectors[0][1] = vector[1];
		smallestVectors[0][2] = vector[2];
		smallestVectors[0][3] = sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]);
		for(j = y+1; j<vReturn->size-1; j++){
			CVectorGetElement(vReturn, &foundpoint, j);
			vector = (double *) foundpoint;
			a1 = acos((vector[0]*smallestVectors[0][0]+vector[1]*smallestVectors[0][1]+vector[2]*smallestVectors[0][2])/(smallestVectors[0][3]*sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2])));
			a1 = a1*180/M_PI;
			if(a1 > 80 && a1 < 100){
				smallestVectors[1][0] = vector[0];
				smallestVectors[1][1] = vector[1];
				smallestVectors[1][2] = vector[2];
				smallestVectors[1][3] = sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]);
				for(x = j+1; x < vReturn->size; x++){
					CVectorGetElement(vReturn, &foundpoint, x);
					vector = (double *) foundpoint;
					a1 = acos((vector[0]*smallestVectors[0][0]+vector[1]*smallestVectors[0][1]+vector[2]*smallestVectors[0][2])/(smallestVectors[0][3]*sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2])));
					a2 = acos((vector[0]*smallestVectors[1][0]+vector[1]*smallestVectors[1][1]+vector[2]*smallestVectors[1][2])/(smallestVectors[1][3]*sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2])));
					a1 = a1*180/M_PI;
					a2 = a2*180/M_PI;
					if(a1 > 80 && a1 < 100 && a2 > 80 && a2 < 100){
						smallestVectors[2][0] = vector[0];
						smallestVectors[2][1] = vector[1];
						smallestVectors[2][2] = vector[2];
						smallestVectors[2][3] = sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]);
						i = 3;
						printf("Found\n");
						break;
					}
				}
			}
			if(i == 3) break;
		}
		if(i == 3) break;
	}
	//Print results
	/*printf("%g %g %g\n", smallestVectors[0][0], smallestVectors[0][1], smallestVectors[0][2]);
	printf("%g %g %g\n", smallestVectors[1][0], smallestVectors[1][1], smallestVectors[1][2]);
	printf("%g %g %g\n", smallestVectors[2][0], smallestVectors[2][1], smallestVectors[2][2]);
	//Find angles between the 3 vectors
	a3 = acos((smallestVectors[1][0]*smallestVectors[0][0]+smallestVectors[1][1]*smallestVectors[0][1]+smallestVectors[1][2]*smallestVectors[0][2])/(smallestVectors[0][3]*smallestVectors[1][3]));
	a2 = acos((smallestVectors[0][0]*smallestVectors[2][0]+smallestVectors[0][1]*smallestVectors[2][1]+smallestVectors[0][2]*smallestVectors[2][2])/(smallestVectors[2][3]*smallestVectors[0][3]));
	a1 = acos((smallestVectors[1][0]*smallestVectors[2][0]+smallestVectors[1][1]*smallestVectors[2][1]+smallestVectors[1][2]*smallestVectors[2][2])/(smallestVectors[2][3]*smallestVectors[1][3]));
	a1 = a1*180/M_PI;
	a2 = a2*180/M_PI;
	a3 = a3*180/M_PI;
	//Print angles
	printf("%g %g %g\n", a1, a2, a3);*/
	//Free arrays
	free(nums);
	free(arr);
	free(arr2);
	for(i=0; i<nelem/2; i++) free(spots[i]);
	free(spots);
	//Free trees and vector
	CNearTreeFree(&tree);
	CNearTreeFree(&tree2);
	CNearTreeFree(&temptree);
	CVectorFree(&vReturn);
	//Close files
	fclose(fp);
	fclose(f);
	return 0;
}
