from __future__ import division
import sys
from math import exp

POSITIVE_CONSTANT = 2


def make_simu_data (patient_file, last_epoch, alpha, variance_gaussian_neighborhood, l2, l2_start_epoch, maps, assocs):
	
	patient = open(patient_file, "w")

	phase_firstepochs = []
	map_alphas = {m : [] for m in maps}
	map_ncs = {m : [] for m in maps}
	map_running = {m : [] for m in maps}

	assoc_alphas = {a : [] for a in assocs}
	assoc_running = {a : [] for a in assocs}

	i = 1

	while i < last_epoch:
		if i == 1:
			phase_firstepochs.append("phase-firstepochs")
			for m in maps:
				map_alphas[m].append("".join([m, "-alphas"]))
				map_ncs[m].append(m + "-ncs")
				map_running[m].append(m + "-running")
			for a in assocs:
				assoc_running[a].append(a + "-assoc-running")
				assoc_alphas[a].append(a + "-assoc-alphas")
		phase_firstepochs.append(i)
		for m in maps:
			map_alphas[m].append(alpha)
			if m== 12:
				if i < l2_start_epoch:
					map_ncs[m].append(0)
				else:
					map_ncs[m].append(variance_gaussian_neighborhood*(exp((0-(i+1-l2_start_epoch))/POSITIVE_CONSTANT)))
			else:
			 	map_ncs[m].append(variance_gaussian_neighborhood*(exp((0-i)/POSITIVE_CONSTANT)))

			if m == l2 and i + 155 < l2_start_epoch:
				map_running[m].append(0)
			else:
			 	map_running[m].append(1)
		for a in assocs:
			assoc_alphas[a].append(alpha)
			if l2 in a and i < l2_start_epoch:
				assoc_running[a].append(0)
			else: assoc_running[a].append(1)

		if (i < l2_start_epoch and abs(i - l2_start_epoch) < 155):
			phase_firstepochs.append(l2_start_epoch)
			# append stuff for maps and assocs here
			for m in maps:
				map_alphas[m].append(alpha)
				if m == l2:
					map_ncs[m].append(variance_gaussian_neighborhood)
				else:
					map_ncs[m].append(variance_gaussian_neighborhood*(exp(-l2_start_epoch/POSITIVE_CONSTANT)))
				
				map_running[m].append(1)
			for a in assocs:
				assoc_alphas[a].append(alpha)
				assoc_running[a].append(1)
		i = i + 155

	phase_firstepochs.append(last_epoch+1)
	patient.write("{}\n".format("\t".join([str(x) for x in phase_firstepochs])))
	
	for m in map_alphas:
		patient.write("{0}\n".format("\t".join([str(x) for x in map_alphas[m]])))
	for a in assoc_alphas:
		patient.write("{0}\n".format("\t".join([str(x) for x in assoc_alphas[a]])))

	for m in map_ncs:
		patient.write("{0}\n".format("\t".join([str(x) for x in map_ncs[m]])))

	for m in map_running:
		patient.write("{0}\n".format("\t".join([str(x) for x in map_running[m]])))
	for a in assoc_running:
		patient.write("{0}\n".format("\t".join([str(x) for x in assoc_running[a]])))

	patient.close()

if __name__ == "__main__":
	make_simu_data("UTBA18.txt", 1400, 0.25, 3, "l2", 180, ["l1", "l2", "sem"], ["l1l2", "sl1", "sl2"])














