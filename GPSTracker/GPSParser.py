#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Nov  9 11:48:42 2020

@author: matthewsolt

Note that the GPS output data assumes north and west hemispheres
"""

#To Do: Fix the output for GPS Coordinates

import sys
tmpargv = sys.argv
sys.argv = []
import getopt
sys.argv = tmpargv
import matplotlib.pyplot as plt 
import csv

#List arguments
def print_usage():
    print("\nUsage: {0} <output file base name> <input csv file>".format(sys.argv[0]))
    print("Arguments: ")
    print('\t-h: this help message')
    
options, remainder = getopt.gnu_getopt(sys.argv[1:], 'h')

# Parse the command line arguments
for opt, arg in options:
    if opt=='-h':
        print_usage()
        sys.exit(0)
        
def stringtrunk(string, length):
    return (string[0+i:length+i] for i in range(0, len(string), length))

acc = [] 
hum = [] 
temp = [] 

nbins = 100

outfile = str(remainder[0])
infile = str(remainder[1])

outcsv = "{0}.csv".format(outfile)
print("hello")

with open(outcsv, 'w', newline = "") as datafile:
    writer = csv.writer(datafile)
    writer.writerow(['Time', 'Temperature', 'Humidity', 'X', 'Y', 'Z', 
                    'Mag', 'Lat', 'Long', 'Speed'])
    file = open(infile, "r")
    for line in file:
        column = line.split()
        if(len(column) != 11): continue
        time = "{0} {1}".format(column[0], column[1])
        temperature = column[2]
        humidity = column[3]
        x = column[4]
        y = column[5]
        z = column[6]
        mag = column[7]
        latlist = list(stringtrunk(column[8], 1))
        longlist = list(stringtrunk(column[9], 1))
        print(longlist)
        print(latlist)
        lat = "{0} N".format(column[8])
        long = "{0} W".format(column[9])
        speed = column[10]
        writer.writerow([time, temperature, humidity, x, y, z, mag, lat, long, speed])
    
    file.close()

with open(outcsv, 'r', newline = "") as datafile: 
    reader = csv.DictReader(datafile) 
    for row in reader: 
        acc.append(float(row['Mag'])) 
        hum.append(float(row['Humidity'])) 
        temp.append(float(row['Temperature'])) 
        
fig0, (ax0, ax1) = plt.subplots(1, 2) 

ax0.hist(acc, nbins, range=[0, max(acc)+5], label = "") 
ax0.set_title("Acceleration")
ax0.set_xlabel("Acceleration (g's)") 
ax0.set_ylabel("") 

y0, x0, _ = ax1.hist(acc, nbins, range=[0, max(acc)+5], label = "") 
ax1.set_title("Acceleration") 
ax1.set_xlabel("Acceleration (g's)") 
ax1.set_ylabel("") 
ax1.set_yscale("log")

ax1.set_ylim([0.5, y0.max() * 10])

plt.savefig('{0}_acc.png'.format(outfile)) 

fig1, (ax0, ax1) = plt.subplots(1, 2) 

ax0.hist(hum, nbins, range=[min(hum) - 5, max(hum) + 5], label = "")
ax0.set_title("Humidity")
ax0.set_xlabel("Relative Humidity (%)")
ax0.set_ylabel("")

y1, x1, _ = ax1.hist(hum, nbins, range=[min(hum) - 5, max(hum) + 5], label = "")
ax1.set_title("Humidity")
ax1.set_xlabel("Relative Humidity (%)") 
ax1.set_ylabel("")
ax1.set_yscale("log")

ax1.set_ylim([0.5,  y1.max() * 10])

plt.savefig('{0}_hum.png'.format(outfile))

fig2, (ax0, ax1) = plt.subplots(1, 2)

ax0.hist(temp, nbins, range=[min(temp) - 5, max(temp) + 5], label = "")
ax0.set_title("Temperature")
ax0.set_xlabel("Temperature (C)")
ax0.set_ylabel("")

y2, x2, _ = ax1.hist(temp, nbins, range=[min(temp) - 5, max(temp) + 5], label = "")
ax1.set_title("Temperature")
ax1.set_xlabel("Temperature (C)")
ax1.set_ylabel("")
ax1.set_yscale("log")

ax1.set_ylim([0.5,  y2.max() * 10])

plt.savefig('{0}_temp.png'.format(outfile))

        
        
        
