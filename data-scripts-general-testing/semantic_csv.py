import csv


with open ('semantic.csv', 'rb') as semantic_file: 
	reader = csv.reader(semantic_file, delimiter = ',')
	f = open ('semantic_reps.txt', 'w')
	reader.next() # skip header
	for row in reader: 
		row[0] = row[0].lower()
		f.write(" ".join(row))
		f.write("\n")
	f.close()

