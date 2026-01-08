import bibtexparser

with open("references.bib") as bibtex_file:
    bib_database = bibtexparser.load(bibtex_file)

for entry in bib_database.entries:
    print(f"Title: {entry['title']}")
    print(f"Authors: {entry['author']}")
