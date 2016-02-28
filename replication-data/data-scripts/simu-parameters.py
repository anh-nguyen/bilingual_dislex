import sys



def make_simu_data (file, last_epoch, alpha, variance_gaussian_neighborhood, l2, l2_start_epoch, maps, assocs):
	
	patient = open(file, "w")

	phase_firstepochs = []
	map_alphas = {m : [] for m in [x + "-alphas" for x in maps]}
	map_ncs = {m : [] for m in [x + "-ncs" for x in maps]}
	map_running = {m : [] for m in [x + "-running" for x in maps]}

	assoc_alphas = {a : [] for a in [x + "-assoc-alphas" for x in assocs]}
	assoc_running = {a : [] for a in [x + "-assoc-running" for x in assocs]}

	for (i = 1; i < last_epoch; i+=155):
		phase_firstepochs.append[i]
		for m in maps:
			map_alphas[m].append(alpha)
			map_ncs[m].append(_______)
			if m == l2 and i + 155 < l2_start_epoch:
				map_running[m].append(0)
			else map_running[m].append(1)
		for a in assocs:
			assoc_alphas[a].append(alpha)
			if l2 in a and i < l2_start_epoch:
				assoc_running[a].append(0)
			else assoc_running[a].append(1)

		if (i != l2_start_epoch and i + 155 > l2_start_epoch):
			phase_firstepochs.append[l2_start_epoch]
			# append stuff for maps and assocs here


if __name__ == "__main__":
	make_simu_data("UTBA18.txt", 1400, 0.25, 3, 180, "l2", ["l1", "l2", "sem"], ["l1l2", "sl1", "sl2"])














