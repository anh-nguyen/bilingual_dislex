import sys

# additional features: 
#	MANNER:					[ for consonants ]
#		tap/flap = 0.100
# 		trill = 0.200
#	CONSONANT LATERALIZATION:
#		not lateral = 0.000
#		lateral = 0.500
#	VOWEL ROUNDNESS: 
#		unrounded = 0.500
#		rounded = 1.000
#	VOWEL LENGTH: 
#		no info / context-dependent = 0.000
#		short = 0.500
#		long = 1.000


# TODO: 
# add rule that averages 2 consonants when there are more than 3 consonants


# The order of feature vectors for CONSONANTS are:
#		'PLACE 		MANNER		VOICEDNESS/SOUND	LATERALIZATION'

consonant_reps = {'_' : '0.000 0.000 0.000 0.000',
				  'G' : '0.875 0.333 0.350 0.000', 
				  'm' : '0.125 0.833 0.250 0.000',
				  'F' : '0.250 0.833 0.250 0.000',
				  'n' : '0.500 0.833 0.250 0.000',
				  'N' : '0.875 0.833 0.250 0.000',
				  'p' : '0.125 0.167 0.750 0.000',
				  'b' : '0.125 0.167 0.250 0.000',
				  't' : '0.500 0.167 0.750 0.000',
				  'd' : '0.500 0.167 0.250 0.000',
				  'c' : '0.750 0.167 0.750 0.000',
				  'k' : '0.875 0.167 0.750 0.000',
				  'g' : '0.875 0.167 0.250 0.000',
				  's' : '0.500 0.333 0.750 0.000',
				  'z' : '0.500 0.333 0.250 0.000',
				  'S' : '0.625 0.333 0.750 0.000',
				  'B' : '0.125 0.333 0.250 0.000',
				  'f' : '0.250 0.333 0.750 0.000',
				  'v' : '0.250 0.333 0.250 0.000',
				  'T' : '0.375 0.333 0.750 0.000',
				  'D' : '0.375 0.333 0.250 0.000',
				  'C' : '0.625 0.333 0.750 0.000',		# tS in SAMPA; translated to one-char length
				  'J' : '0.750 0.333 0.250 0.000',		# j\ in SAMPA; translated to one-char length
				  'j' : '0.750 0.500 0.250 0.000',
				  'x' : '0.875 0.333 0.750 0.000',
				  'h' : '1.000 0.333 0.750 0.000',
				  '4' : '0.500 0.100 0.250 0.000',
				  'r' : '0.500 0.200 0.250 0.000',
				  'l' : '0.500 0.667 0.250 0.500',
				  'L' : '0.750 0.667 0.250 0.000',
				  'w' : '0.125 0.500 0.250 0.000'
				  }


# The order of feature vectors for VOWELS are:
#	'SONORITY/HEIGHT	CHROMACITY/TONGUE POSITION 	 	LENGTH 		ROUNDNESS'

vowel_reps = {'i' : '0.143 0.200 0.000 0.500',
			  'y' : '0.143 0.200 0.000 1.000', 
			  'u' : '0.143 1.000 0.000 1.000', 
			  'I' : '0.286 0.400 0.500 0.500', 
			  'U' : '0.286 0.800 0.000 1.000', 
			  'e' : '0.286 0.200 0.000 0.500', 
			  'o' : '0.286 1.000 0.000 1.000', 
			  '@' : '0.571 0.600 0.000 0.500', 
			  'E' : '0.714 0.200 0.500 0.500', 
			  'V' : '0.714 1.000 0.500 0.500', 
			  'O' : '0.714 1.000 0.000 1.000', 
			  '{' : '0.857 0.200 0.500 0.500', 
			  'a' : '1.000 0.200 0.000 0.500', 
			  'A' : '1.000 1.000 0.000 0.500', 
			  }

# decide where the padding for English & Spanish will be
# 		for English, the primary stress syllable should be at index 20 of an array / symbol index 4 (i.e. 5th pos.)
#	 	for Spanish, the primary stress syllable should be at index 50 of an array / symbol index 10 (i.e. 11th pos.)
# translate everything to numbers
# pad left for stress
# when finish, pad right to equal the longest one


# stress indexes figured out manually by looking at the word where index of the primary stress character is highest
ENG_STRESS_INDEX = 6       
SPANISH_STRESS_INDEX = 16

# max length figured out by the find_max_length and test_max_length variables in num-reps
ENG_MAX_LENGTH = 30
SPANISH_MAX_LENGTH = 52


def num_reps (sampa_file, padded_sampa_file, result_file, stress_index, max_length):
	find_max_length = True
	sampa = open(sampa_file, "r")
	syl = open(padded_sampa_file, "r")
	rep = open(result_file, "w")
	test_max_length = 0
	longest_word = ""

	for word in sampa:
		consonants_between_vowels = 0
		word = word.strip() 			# remove \n and space
		word = word.replace(" ", "_")
		padded_word = syl.readline().strip()
		try:
			word_stress_index = padded_word.index('"')
		except ValueError: 
			word_stress_index = 0
		padded_word = padded_word.replace("j\\", "J")			# make the symbol j\ one-character long
		padded_word = padded_word.replace("tS", "C")			# make the symbol tS one-character long
		padded_word = padded_word.replace("\"", "")			# remove primary stress symbols
		padded_word = padded_word.replace("%", "")			# remove secondary stress symbols

		numeric_rep = []

		# fill in with blanks so that primary stress lines up
		for _ in range(0, stress_index - word_stress_index):
			numeric_rep.append(consonant_reps['_'])

		for symbol in padded_word:
			# fill word in with actual symbols
			if symbol in consonant_reps:
				numeric_rep.append(consonant_reps[symbol])
				consonants_between_vowels = consonants_between_vowels + 1
				if consonants_between_vowels == 5:
					print "3-consonant word = {0} \n".format(padded_word)
			elif symbol in vowel_reps:
				numeric_rep.append(vowel_reps[symbol])
				consonants_between_vowels = 0
			else:
				sys.exit("Symbol {0} from {1} not in dict".format(symbol, padded_word))

		# right-pad to match length of longest word
		if len(numeric_rep) > test_max_length and find_max_length:			# code for finding out max length
			test_max_length = len(numeric_rep)
			longest_word = word

		# right_padding
		for _ in range(len(numeric_rep), max_length):
			numeric_rep.append(consonant_reps["_"])


		rep.write("{0} {1} \n".format(word, " ".join(numeric_rep)))

	if find_max_length:
		if stress_index == ENG_STRESS_INDEX:
			print("English longest string = {0} {1}; nreps = {2}".format(test_max_length, longest_word, test_max_length*4))
		else:
			print("Spanish longest string = {0} {1}; nreps = {2}".format(test_max_length, longest_word, test_max_length*4))

	sampa.close()
	syl.close()
	rep.close()


if __name__ == "__main__":
	num_reps("en_sampa.txt", "en_sampa_syllables.txt", "l1.txt", ENG_STRESS_INDEX, ENG_MAX_LENGTH)
	num_reps("es_sampa.txt", "es_sampa_syllables.txt", "l2.txt", SPANISH_STRESS_INDEX, SPANISH_MAX_LENGTH)


