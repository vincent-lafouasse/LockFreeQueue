import bibtexparser
import re

with open("references.bib") as bibtex_file:
    bib_database = bibtexparser.load(bibtex_file)

parsed_data = []

for entry in bib_database.entries:
    raw_authors = entry.get("author", "")
    author_list = re.split(r"\s+and\s+", raw_authors, flags=re.IGNORECASE)
    author_list = [author.strip() for author in author_list]
    parsed_data.append(
        {"title": entry.get("title", "Unknown Title"), "authors": author_list}
    )

for item in parsed_data:
    print(f"Title:\n    {item['title']}")
    print(f"Authors:")
    for author in item['authors']:
        print(f"    {author}")
    print()
