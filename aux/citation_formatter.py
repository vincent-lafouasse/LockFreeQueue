import bibtexparser
import re

with open("references.bib") as bibtex_file:
    bib_database = bibtexparser.load(bibtex_file)


class Author:
    def __init__(self, first_names, last_name):
        # first_names is expected to be a list of strings
        self.first_names = first_names
        self.last_name = last_name

    @classmethod
    def parse(cls, name_string):
        name_string = name_string.strip()

        if "," in name_string:
            # Format: Last, First Middle
            parts = name_string.split(",")
            last_name = parts[0].strip()
            first_names = parts[1].strip().split()
        else:
            # Format: First Middle Last
            parts = name_string.split()
            if len(parts) > 1:
                last_name = parts[-1]
                first_names = parts[:-1]
            else:
                last_name = parts[0]
                first_names = []

        return cls(first_names, last_name)

    def jacs(self):
        # Format: Last, F. M.
        initials = " ".join([f"{n[0]}." for n in self.first_names])
        return f"{self.last_name}, {initials}"

    def __repr__(self):
        return f"Author({self.last_name}, {self.first_names})"


class Reference:
    def __init__(self):
        self.title = None
        self.authors = None


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
    for author in item["authors"]:
        print(f"    {author}")
    print()
