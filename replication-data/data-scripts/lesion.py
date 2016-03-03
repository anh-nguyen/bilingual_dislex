from __future__ import division
import sys
from random import gauss



def lesion_assocs (patient_file, l1_map_size, l2_map_size, sem_map_size, noise_amount):
	
	patient = open(patient_file, "w")

	line_num = 0

	for line in patient:				# read until we get to network weights
		line_num += 1
		if "network-weights" in line:
			break

	l1l2_start = 
	l1l2_exclusive_end = 
	sl1_start = 
	sl1_exclusive_end =
	sl2_start =
	sl2_exclusive_end =



if __name__ == "__main__":
	UTBA18_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}
	BUBA01_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}
	UTBA20_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}
	UTBA21_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}

	lesion_assocs()









