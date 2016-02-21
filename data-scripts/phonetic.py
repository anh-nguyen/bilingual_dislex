import sys
# additional features: 
# 	tap/flap = manner, 0.100
# 	trill = manner, 0.200


# separate consonant and vowel dict
# separate features for each (still 4 per letter though - add features as mentioned in paper)
# add rule that averages 2 consonants when there are more than 3 consonants

phonetic_features_dict = {'_' : '0.000 0.000 0.000 0.000 0.000',
						  'G' : '0.875 0.333 0.350 0.000 0.000', 
						  'm' : '0.125 0.833 0.250 0.000 0.000',
						  'F' : '0.250 0.833 0.250 0.000 0.000',
						  'n' : '0.500 0.833 0.250 0.000 0.000',
						  'N' : '0.875 0.833 0.250 0.000 0.000',
						  'p' : '0.125 0.167 0.750 0.000 0.000',
						  'b' : '0.125 0.167 0.250 0.000 0.000',
						  't' : '0.500 0.167 0.750 0.000 0.000',
						  'd' : '0.500 0.167 0.250 0.000 0.000',
						  'c' : '0.750 0.167 0.750 0.000 0.000',
						  'k' : '0.875 0.167 0.750 0.000 0.000',
						  'g' : '0.875 0.167 0.250 0.000 0.000',
						  's' : '0.500 0.333 0.750 0.000 0.000',
						  'z' : '0.500 0.333 0.250 0.000 0.000',
						  'S' : '0.625 0.333 0.750 0.000 0.000',
						  'B' : '0.125 0.333 0.250 0.000 0.000',
						  'f' : '0.250 0.333 0.750 0.000 0.000',
						  'v' : '0.250 0.333 0.250 0.000 0.000',
						  'T' : '0.375 0.333 0.750 0.000 0.000',
						  'D' : '0.375 0.333 0.250 0.000 0.000',
						  'C' : '0.625 0.333 0.750 0.000 0.000',		# tS in SAMPA; translated to one-char
						  'J' : '0.750 0.333 0.250 0.000 0.000',		# j\ in SAMPA; translated to one-character length
						  'j' : '0.750 0.500 0.250 0.000 0.000',
						  'x' : '0.875 0.333 0.750 0.000 0.000',
						  'h' : '1.000 0.333 0.750 0.000 0.000',
						  '4' : '0.500 0.100 0.250 0.000 0.000',
						  'r' : '0.500 0.200 0.250 0.000 0.000',
						  'l' : '0.500 0.667 0.250 0.000 0.000',
						  'L' : '0.750 0.667 0.250 0.000 0.000',
						  'w' : '0.125 0.500 0.250 0.000 0.000',
						  'i' : '0.000 1.000 0.250 0.200 0.143',	# vowels start here
						  'y' : '0.000 1.000 0.250 0.200 0.143', 
						  'u' : '0.000 1.000 0.250 1.000 0.143', 
						  'I' : '0.000 1.000 0.250 0.400 0.286', 
						  'U' : '0.000 1.000 0.250 0.800 0.286', 
						  'e' : '0.000 1.000 0.250 0.200 0.286', 
						  'o' : '0.000 1.000 0.250 1.000 0.286', 
						  '@' : '0.000 1.000 0.250 0.600 0.571', 
						  'E' : '0.000 1.000 0.250 0.200 0.714', 
						  'V' : '0.000 1.000 0.250 1.000 0.714', # some are the same b/c rounded/unrounded not encoded...?
						  'O' : '0.000 1.000 0.250 1.000 0.714', 
						  '{' : '0.000 1.000 0.250 0.200 0.857', 
						  'a' : '0.000 1.000 0.250 0.200 1.000', 
						  'A' : '0.000 1.000 0.250 1.000 1.000', 
						  }



# decide where the padding for English & Spanish will be
# 		for English, the primary stress syllable should be at index 20 of an array / symbol index 4 (i.e. 5th pos.)
#	 	for Spanish, the primary stress syllable should be at index 50 of an array / symbol index 10 (i.e. 11th pos.)
# translate everything to numbers
# pad left for stress
# when finish, pad right to equal the longest one


# stress indexes figured out manually by looking at the word where index of the primary stress character is highest
english_stress_index = 6       
spanish_stress_index = 20

def num_reps (sampa_file, result_file, stress_index, max_length):
	find_max_length = True
	sampa = open(sampa_file, "r")
	rep = open(result_file, "w")
	test_max_length = 0

	for word in sampa:
		word = word.strip() 			# remove \n and space
		word = word.replace(" ", "_")
		try:
			word_stress_index = word.index('"')
		except ValueError: 
			word_stress_index = 0
		new_word = word.replace("j\\", "J")			# make the symbol j\ one-character long
		new_word = new_word.replace("tS", "C")			# make the symbol tS one-character long
		new_word = new_word.replace("\"", "")			# remove primary stress symbols
		new_word = new_word.replace("%", "")			# remove secondary stress symbols

		numeric_rep = []

		# fill in with blanks so that primary stress lines up
		for _ in range(0, stress_index - word_stress_index):
			numeric_rep.append(phonetic_features_dict['_'])

		for symbol in new_word:

			# fill word in with actual symbols
			if symbol not in phonetic_features_dict:
				sys.exit("Symbol {0} from {1} not in dict".format(symbol, new_word))
			else: 
				numeric_rep.append(phonetic_features_dict[symbol])

		rep.write("{0} {1} \n".format(word, " ".join(numeric_rep)))

	sampa.close()
	rep.close()


if __name__ == "__main__":
	num_reps("en_sampa.txt", "l1.txt", english_stress_index, eng_max_length)
	num_reps("es_sampa.txt", "l2.txt", spanish_stress_index, spanish_max_length)


