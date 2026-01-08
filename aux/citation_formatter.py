import bibtexparser
import re

# required fields for _all_documents
COMMON_REQUIRED = {
    "author",
    "title",
    "year",
    "archive",
}

# type-specific, e.g. journal for an article
# mostly IEEE compliant (i skipped the location field for proceedings)
TYPE_SPECIFIC_REQUIRED = {
    "article": {
        "journal",
        "volume",
        "pages",
        "number",  # issue
        "month",
        "doi",
    },
    "inproceedings": {
        "booktitle",
        "pages",
        "doi",
    },
    "misc": set(),  # maybe howpublished later but for now, nothing
}


def validate_entry(entry):
    entry_type = entry.get("ENTRYTYPE").lower()

    fields = set(entry.keys())

    # set operations
    missing_common_fields = COMMON_REQUIRED - fields

    specific_fields = TYPE_SPECIFIC_REQUIRED.get(entry_type)
    if specific_fields is None:
        print(f"Unrecognised entry type: {entry_type}. No required fields")
        specific_fields = set()

    missing_specific_fields = specific_fields - fields

    valid = True

    for missing in missing_common_fields:
        print(f"{entry.get("ID")}: missing common field:\t{missing}")
        valid = False

    for missing in missing_specific_fields:
        print(f"{entry.get("ID")}: missing specific field:\t{missing}")
        valid = False

    if valid:
        valid_str = "ok"
    else:
        valid_str = "ko"
    print(f"---- {entry.get("ID")}: {valid_str}")
    return valid


def validate_library(library):
    invalid_entries = [
        entry.get("ID") for entry in library.entries if not validate_entry(entry)
    ]

    if len(invalid_entries) != 0:
        raise ValueError("Malformed library")


class Markdown:
    @staticmethod
    def bold(text):
        return f"**{text}**" if text else ""

    @staticmethod
    def it(text):
        return f"*{text}*" if text else ""

    @staticmethod
    def link(url, text=None):
        if not text:
            text = url
        return f"[{text}]({url})"

    @staticmethod
    def sup(text):
        return f"^{text}^" if text else ""


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

    def ieee(self) -> str:
        """Format: F. M. Last"""
        if not self.first_names:
            return self.last_name
        initials = " ".join([f"{n[0]}." for n in self.first_names])
        return f"{initials} {self.last_name}"

    def __repr__(self):
        return f"Author({self.last_name}, {self.first_names})"


class AuthorList:
    def __init__(self, author_list):
        self.author_list = author_list

    @classmethod
    def parse(cls, string):
        author_list = re.split(r"\s+and\s+", string, flags=re.IGNORECASE)
        author_list = [author.strip() for author in author_list]
        author_list = [Author.parse(author) for author in author_list]
        return cls(author_list)

    def ieee(self):
        formatted_names = [a.ieee() for a in self.author_list]
        count = len(formatted_names)

        if count == 0:
            return ""
        if count == 1:
            return formatted_names[0]
        if count == 2:
            return f"{formatted_names[0]} and {formatted_names[1]}"

        # for 3+, use Oxford comma per IEEE
        return ", ".join(formatted_names[:-1]) + f", and {formatted_names[-1]}"


class Reference:
    def __init__(self, entry):
        self.mnemonic = entry.get("ID")
        self.ref_type = entry.get("ENTRYTYPE")

        self.year = entry.get("year")

        self.set_title(entry.get("title", None))
        self.authors = Reference.parse_authors(entry.get("author", ""))

    def set_title(self, title):
        title = title.strip()
        if title[-1] != ".":
            title += "."
        self.title = title

    @staticmethod
    def parse_authors(string):
        author_list = re.split(r"\s+and\s+", string, flags=re.IGNORECASE)
        author_list = [author.strip() for author in author_list]
        return [Author.parse(author) for author in author_list]

    def format(self):
        authors = "; ".join(author.jacs() for author in self.authors)

        return f"{authors} {self.title}"

    def log(self):
        print(f"{self.mnemonic} ({self.ref_type}) {{")
        print(f"    Title:")
        print(f"        {self.title}")
        print(f"    Authors:")
        for author in self.authors:
            print(f"        {author}")
        print()


# a double sanity check that the field exists
def get_field(entry, field):
    mnemonic = entry.get("ID")
    out = entry.get(field)
    if out is None or not str(out).strip():
        raise ValueError(f"{mnemonic} has no field {field}")
    return out


def format_entry(entry) -> str:
    mnemonic = get_field(entry, "ID")
    etype = get_entry(entry, "ENTRYTYPE")

    authors = AuthorList.parse(get_entry(entry, "author"))
    title = get_entry(entry, "title")
    year = get_entry(entry, "year")
    archive = get_entry(entry, "archive")


def main():
    with open("references.bib") as bibtex_file:
        library = bibtexparser.load(bibtex_file)

    validate_library(library)

    references = [Reference(entry) for entry in library.entries]

    for ref in references:
        ref.log()
        print(ref.format())
        print()


if __name__ == "__main__":
    main()
