from __future__ import division
import sys
from random import gauss
import os.path

GAUSSIAN_MEAN = 0

def lesion_assocs (patient_file, lesion_file, map_sizes, num_reps, noise_amount, overwrite):
	
	if os.path.isfile(lesion_file) and not overwrite:
		print ("lesion_file {} already exists... not overwriting\n".format(lesion_file))
		return
	patient = open(patient_file, "r")
	lesioned = open(lesion_file, "w")

	line_num = 0

	for line in patient:				# read until we get to network weights
		line_num += 1
		lesioned.write(line)
		if "network-weights" in line:
			break

	assocs_start = line_num + (map_sizes["l1"]**2)*num_reps["l1"] + (map_sizes["l2"]**2)*num_reps["l2"] + (map_sizes["sem"]**2)*num_reps["sem"] + 1

	# order in the file: sl1 assocs, sl2 assocs, l1l2 assocs

	sl1_start = assocs_start
	sl2_start = sl1_start + (map_sizes["l1"]**2)*(map_sizes["sem"]**2)*2 + 1
	l1l2_start = sl2_start + (map_sizes["l2"]**2)*(map_sizes["sem"]**2)*2 + 1
	l1l2_inclusive_end = l1l2_start + (map_sizes["l1"]**2)*(map_sizes["l2"]**2)*2

	# iterate until we reach assoc weights
	for line in patient:	# starting from network-weights
		line_num += 1
		lesioned.write(line)
		if line_num == assocs_start - 1:
			break

	# add noise to sl1 weights
	for line in patient:
		line_num += 1
		orig = float(line.strip())
		with_noise = orig + gauss(GAUSSIAN_MEAN, noise_amount["sl1"])
		with_noise = with_noise if with_noise > 0 else 0
		lesioned.write("{:.6f}\n".format(with_noise))
		if line_num == sl2_start - 1:
			break

	for line in patient:
		line_num += 1
		orig = float(line.strip())
		with_noise = orig + gauss(GAUSSIAN_MEAN, noise_amount["sl2"])
		with_noise = with_noise if with_noise > 0 else 0
		lesioned.write("{:.6f}\n".format(with_noise))
		if line_num == l1l2_start - 1:
			break

	for line in patient:
		line_num += 1
		orig = float(line.strip())
		with_noise = orig + gauss(GAUSSIAN_MEAN, noise_amount["l1l2"])
		with_noise = with_noise if with_noise > 0 else 0
		lesioned.write("{:.6f}\n".format(with_noise))
		if line_num == l1l2_inclusive_end:
			break

if __name__ == "__main__":
	map_sizes = {"l1": 20, "l2": 20, "sem": 20}
	num_reps = {"l1": 208, "l2": 120, "sem": 81}

	UTBA18_noise_amount = {'l1l2': 0, 'sl1': 0.32, 'sl2': 0.33}
	BUBA01_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}
	UTBA20_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}
	UTBA21_noise_amount = {'l1l2': 0, 'sl1': 0, 'sl2': 0}

	UTBA18_overwrite = True
	BUBA01_overwrite = False
	UTBA20_overwrite = False
	UTBA21_overwrite = False

	lesion_assocs("../../UTBA18-simu", "../../UTBA18-lesioned-simu", map_sizes, num_reps, UTBA18_noise_amount, UTBA18_overwrite)
	#lesion_assocs("../../BUBA01-simu", "../../BUBA01-lesioned-simu", map_sizes, num_reps, BUBA01_noise_amount, BUBA01_overwrite)
	#lesion_assocs("../../UTBA20-simu", "../../UTBA20-lesioned-simu", map_sizes, num_reps, UTBA20_noise_amount, UTBA20_overwrite)
	#lesion_assocs("../../UTBA21-simu", "../../UTBA21-lesioned-simu", map_sizes, num_reps, UTBA21_noise_amount, UTBA21_overwrite)









