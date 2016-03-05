from __future__ import division
import sys
from math import exp, ceil, sqrt

# Formulas for time constant, neighborhood size, and learning rate decay from:
#	http://chem-eng.utoronto.ca/~datamining/Presentations/SOM.pdf and
# 	http://www.ai-junkie.com/ann/som/som3.html

def make_simu_data (patient_file, last_epoch, alpha, starting_radius, l2, l2_start_epoch, maps, assocs):
	
	time_constant = last_epoch / starting_radius

	patient = open(patient_file, "w")

	phase_firstepochs = []
	map_alphas = {m : [] for m in maps}
	map_ncs = {m : [] for m in maps}
	map_running = {m : [] for m in maps}

	assoc_alphas = {a : [] for a in assocs}
	assoc_running = {a : [] for a in assocs}

	i = 1

	while i < last_epoch:
		alpha_value = alpha if i == 1 else alpha * (155/i)
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
			map_alphas[m].append(alpha_value)
			if m == l2:
				if i < l2_start_epoch:
					map_ncs[m].append(0)
				else:
					map_ncs[m].append(ceil(starting_radius*(exp((0-(i+1-l2_start_epoch))/time_constant))))
			else:
				if i == 1:
					map_ncs[m].append(starting_radius)
			 	else: 
			 		map_ncs[m].append(ceil(starting_radius*(exp((0-i)/time_constant))))

			if m == l2 and i < l2_start_epoch:
				map_running[m].append(0)
			else:
			 	map_running[m].append(1)
		for a in assocs:
			assoc_alphas[a].append(alpha_value)
			if l2 in a and i < l2_start_epoch:
				assoc_running[a].append(0)
			else: assoc_running[a].append(1)

		if (i < l2_start_epoch and abs(i - l2_start_epoch) < 155):
			phase_firstepochs.append(l2_start_epoch)
			# append stuff for maps and assocs here
			for m in maps:
				map_alphas[m].append(alpha_value)
				if m == l2:
					map_ncs[m].append(starting_radius)
				else:
					map_ncs[m].append(ceil(starting_radius*(exp(-l2_start_epoch/time_constant))))
				
				map_running[m].append(1)
			for a in assocs:
				assoc_alphas[a].append(alpha_value)
				assoc_running[a].append(1)
		i = i + 155

	phase_firstepochs.append(last_epoch+1)
	patient.write("{}\n".format("\t".join([str(x) for x in phase_firstepochs])))
	
	for m in maps:
		patient.write("{0}\n".format("\t".join([str(x) for x in map_alphas[m] + [0]])))
	for a in assocs:
		patient.write("{0}\n".format("\t".join([str(x) for x in assoc_alphas[a] + [0]])))

	for m in maps:
		patient.write("{0}\n".format("\t".join([str(x) for x in map_ncs[m] + [0]])))

	for m in maps:
		patient.write("{0}\n".format("\t".join([str(x) for x in map_running[m]])))
	for a in assocs:
		patient.write("{0}\n".format("\t".join([str(x) for x in assoc_running[a]])))

	patient.close()

if __name__ == "__main__":
	make_simu_data("UTBA18.txt", 1400, 0.25, 3, "l2", 180, ["l1", "l2", "sem"], ["l1l2", "sl1", "sl2"])
	make_simu_data("BUBA01.txt", 800, 0.25, 3, "l2", 180, ["l1", "l2", "sem"], ["l1l2", "sl1", "sl2"])
	make_simu_data("UTBA20.txt", 1700, 0.25, 3, "l2", 300, ["l1", "l2", "sem"], ["l1l2", "sl1", "sl2"])
	make_simu_data("UTBA21.txt", 1700, 0.25, 3, "l2", 100, ["l1", "l2", "sem"], ["l1l2", "sl1", "sl2"])










