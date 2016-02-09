#!/usr/bin/python
# Nick McComb | www.nickmccomb.net
# Written Jan 2016 for CS344
# 
# This program writes 3 random 10 character, lowercase strings out to three different files,
# then outputs and multiplies two random ints (between 1 and 42).

import sys
import random

def getRandomString():
  filename = ''
  for i in range(10):
    filename += str(unichr(random.randint(97, 122)))
  return filename

#random.seed(7)

for i in range(1, 4):
  randomStr = getRandomString()
  f = open('file' + str(i), 'w')
  f.write(randomStr)
  print "Wrote " + randomStr + " to file" + str(i)
  f.close()

int1 = random.randint(1, 43)
int2 = random.randint(1, 43)
print "Your first integer is:   " + str(int1)
print "Your second integer is:  " + str(int2)
print "The product of these is: " + str(int1*int2)

#print getRandomString()
