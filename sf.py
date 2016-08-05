from __future__ import division
import math
from PIL import Image
import numpy as np
import bisect

img = Image.open("lyso.pgm")
width, height = img.size
array1 = np.asarray(img)

startx = 1000
starty = 450
endx = 1450
endy = 600

newArray = np.empty(shape=(endy-starty,(endx-startx)))

box = 15
nums = []
n = 0
avg = 0
smos = 0



for y in range(starty, endy):
	left = int(n/10)
	right = n-int(9*n/10)
	b = endy-1
	t = starty
	
	if y%2 == 0:
		if y == starty:
			b = starty + box
			for j in range(starty, starty+box+1):
				for i in range(startx, startx+box+1):
					if array1[j][i] != 0:
						bisect.insort_left(nums,array1[j][i])
						n += 1
			for i in range(int(n/10),int(9*n/10)):
				avg += nums[i]
				smos += int(nums[i])*nums[i]
		else:
			if y-box-1 >= starty:
				t = y-box
				for i in range(startx, startx+box+1):
					val = array1[y-box-1][i]
					if val != 0:
						ind = nums.index(val)
						if ind < left:
							left -= 1
							n -= 1
						elif ind >= n-right:
							right -= 1
							n-= 1
						else:
							n -= 1
							avg -= val
							smos -= int(val)*val
						del nums[ind]
			if y+box < endy:
				b = y+box
				for i in range(startx, startx+box+1):
					val = array1[y+box][i]
					if val != 0:
						bisect.insort_left(nums,val)
						ind = nums.index(val)
						n += 1
						if ind < left:
							left += 1
						elif ind >= n-right:
							right += 1
						else:
							avg += val
							smos += int(val)*val
			if left < int(n/10):
				for i in range(left, int(n/10)):
					avg -= nums[i]
					smos -= int(nums[i])*nums[i]
			elif left > int(n/10):
				for i in range(int(n/10), left):
					avg += nums[i]
					smos += int(nums[i])*nums[i]
			if right < n-int(9*n/10):
				for i in range(int(9*n/10), n-right):
					avg -= nums[i]
					smos -= int(nums[i])*nums[i]
			elif right > n-int(9*n/10):
				for i in range(n-right, int(9*n/10)):
					avg += nums[i]
					smos += int(nums[i])*nums[i]
		for x in range(startx, endx):
			left = int(n/10)
			right = n-int(9*n/10)

			if x == startx:
				pass
			else:
				if x-box-1 >= startx:
					for i in range(t, b+1):
						val = array1[i][x-box-1]
						if val != 0:
							ind = nums.index(val)
							if ind < left:
								left -= 1
								n -= 1
							elif ind >= n-right:
								right -= 1
								n-= 1
							else:
								n -= 1
								avg -= val
								smos -= int(val)*val
							del nums[ind]
				if x+box < endx:
					for i in range(t, b+1):
						val = array1[i][x+box]
						if val != 0:
							bisect.insort_left(nums,val)
							ind = nums.index(val)
							n += 1
							if ind < left:
								left += 1
							elif ind >= n-right:
								right += 1
							else:
								avg += val
								smos += int(val)*val
				if left < int(n/10):
					for i in range(left, int(n/10)):
						avg -= nums[i]
						smos -= int(nums[i])*nums[i]
				elif left > int(n/10):
					for i in range(int(n/10), left):
						avg += nums[i]
						smos += int(nums[i])*nums[i]
				if right < n-int(9*n/10):
					for i in range(int(9*n/10), n-right):
						avg -= nums[i]
						smos -= int(nums[i])*nums[i]
				elif right > n-int(9*n/10):
					for i in range(n-right, int(9*n/10)):
						avg += nums[i]
						smos += int(nums[i])*nums[i]

			sd = math.sqrt(abs(((int(9*n/10)-int(n/10))*smos-avg*avg)/((int(9*n/10)-int(n/10))*(int(9*n/10)-int(n/10)-1))))
			avg = avg/(int(9*n/10)-int(n/10))
			if array1[y][x] > avg+sd*5:
				newArray[y-starty][x-startx] = 0
			else:
				newArray[y-starty][x-startx] = 255
			avg = avg * (int(9*n/10)-int(n/10))
	else:
		if y-box-1 >= starty:
			t = y-box
			for i in range(endx-box-1, endx):
				val = array1[y-box-1][i]
				if val != 0:
					ind = nums.index(val)
					if ind < left:
						left -= 1
						n -= 1
					elif ind >= n-right:
						right -= 1
						n-= 1
					else:
						n -= 1
						avg -= val
						smos -= int(val)*val
					del nums[ind]
		if y+box < endy:
			b = y+box
			for i in range(endx-box-1, endx):
				val = array1[y+box][i]
				if val != 0:
					bisect.insort_left(nums,val)
					ind = nums.index(val)
					n += 1
					if ind < left:
						left += 1
					elif ind >= n-right:
						right += 1
					else:
						avg += val
						smos += int(val)*val
		if left < int(n/10):
			for i in range(left, int(n/10)):
				avg -= nums[i]
				smos -= int(nums[i])*nums[i]
		elif left > int(n/10):
			for i in range(int(n/10), left):
				avg += nums[i]
				smos += int(nums[i])*nums[i]
		if right < n-int(9*n/10):
			for i in range(int(9*n/10), n-right):
				avg -= nums[i]
				smos -= int(nums[i])*nums[i]
		elif right > n-int(9*n/10):
			for i in range(n-right, int(9*n/10)):
				avg += nums[i]
				smos += int(nums[i])*nums[i]
				
		for x in range(endx-1, startx-1, -1):
			left = int(n/10)
			right = n-int(9*n/10)

			if x == endx-1:
				pass
			else:
				if x+box+1<endx:
					for i in range(t, b+1):
						val = array1[i][x+box+1]
						if val != 0:
							ind = nums.index(val)
							if ind < left:
								left -= 1
								n -= 1
							elif ind >= n-right:
								right -= 1
								n-= 1
							else:
								n -= 1
								avg -= val
								smos -= int(val)*val
							del nums[ind]
				if x-box >= startx:
					for i in range(t, b+1):
						val = array1[i][x-box]
						if val != 0:
							bisect.insort_left(nums,val)
							ind = nums.index(val)
							n += 1
							if ind < left:
								left += 1
							elif ind >= n-right:
								right += 1
							else:
								avg += val
								smos += int(val)*val
				if left < int(n/10):
					for i in range(left, int(n/10)):
						avg -= nums[i]
						smos -= int(nums[i])*nums[i]
				elif left > int(n/10):
					for i in range(int(n/10), left):
						avg += nums[i]
						smos += int(nums[i])*nums[i]
				if right < n-int(9*n/10):
					for i in range(int(9*n/10), n-right):
						avg -= nums[i]
						smos -= int(nums[i])*nums[i]
				elif right > n-int(9*n/10):
					for i in range(n-right, int(9*n/10)):
						avg += nums[i]
						smos += int(nums[i])*nums[i]

			sd = math.sqrt(abs(((int(9*n/10)-int(n/10))*smos-avg*avg)/((int(9*n/10)-int(n/10))*(int(9*n/10)-int(n/10)-1))))
			avg = avg/(int(9*n/10)-int(n/10))
			if array1[y][x] > avg+sd*5:
				newArray[y-starty][x-startx] = 0
			else:
				newArray[y-starty][x-startx] = 255
			avg = avg * (int(9*n/10)-int(n/10))

newImage = Image.fromarray(newArray)
newImage = newImage.convert('L')
newImage.save("testSpotFinder.pgm")
