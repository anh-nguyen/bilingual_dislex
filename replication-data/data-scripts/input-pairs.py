import sys

NUMBER_OF_WORDS = 60

def make_input_pairs (l1_file, l2_file, semantic_file, result_file, monolingual_file):
	
	l1 = open(l1_file, "r")
	l2 = open(l2_file, "r")
	sem = open(semantic_file, "r")
	result = open(result_file, "w")
	monolingual = open(monolingual_file, "w")

	for _ in range (0, NUMBER_OF_WORDS): 
		l1_word = l1.readline().split(" ", 2)[0]
		l2_word = l2.readline().split(" ", 2)[0]
		sem_word = sem.readline().split(" ", 2)[0]
		result.write("{0}\t{1}\t{2}\n".format(l1_word, l2_word, sem_word))
		monolingual.write("{0}\t{1}\n".format(l1_word, sem_word))


	l1.close()
	l2.close()
	sem.close()
	result.close()


if __name__ == "__main__":
	make_input_pairs("l1.txt", "l2.txt", "semantic_reps.txt", "input_pairs.txt", "monolingual.txt")














