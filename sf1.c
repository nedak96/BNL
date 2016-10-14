#include <stdio.h>
#include<stdlib.h>
#include <math.h>

//change first number to size of box
int nums[(10*2+1)*(10*2+1)];
int arr[2527][2463];
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
	FILE *fp = fopen("/root/BNL/array.txt", "r");
	int i,j;
	arr[0][0] = 1;
	if(fp!=NULL){
		for(j=0; j<2527; j++){
			for(i=0; i<2463; i++){
				fscanf(fp, "%d ",&arr[j][i]);
			}
		}
	}
	fclose(fp);
	int startx = 1000;
	int starty = 450;
	int endx = 1450;
	int endy = 600;
	long sum = 0;
	int box=10;
	long smos = 0;
	int y, x;
	n = 0;
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
						if(arr[j][i] != 0){
							insert(arr[j][i]);
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
						int val = arr[y-box-1][i];
						if (val != 0){
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
						int val = arr[y+box][i];
						if (val != 0){
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
							int val = arr[i][x-box-1];
							if (val != 0){
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
							int val = arr[i][x+box];
							if (val != 0){
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
				//printf("%f\n", average);
			}	
		}
		else{
			if(y-box-1 >= starty){
				top = y-box;
				for(i=endx-box-1; i< endx; i++){
					int val = arr[y-box-1][i];
					if (val != 0){
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
					int val = arr[y+box][i];
					if (val != 0){
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
							int val = arr[i][x+box+1];
							if (val != 0){
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
							int val = arr[i][x-box];
							if (val != 0){
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
				//printf("%f\n", average);
			}
		}
	}
	return 0;
}
