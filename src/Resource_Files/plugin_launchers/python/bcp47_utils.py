def bcp47_normalize(tag):
    if not tag or not tag.strip(): return tag
    subtags = tag.strip().replace("_", "-").split("-")
    normalized = []
    in_extension_or_private = False
    for i, sub in enumerate(subtags):
        if in_extension_or_private:
            # Everything after a singleton (e.g., 'x', 'u', 't') stays lowercase
            normalized.append(sub.lower())
            continue
        if i == 0:
            # primary language subtag
            normalized.append(sub.lower())
            if sub.lower() == "x": in_extension_or_private = True
        elif len(sub) == 1:
            # single char starting an extension or private-use section
            normalized.append(sub.lower())
            in_extension_or_private = True
        elif len(sub) == 4 and sub.isalpha():
            # script subtag
            normalized.append(sub.title())
        elif (len(sub) == 2 and sub.isalpha()) or (len(sub) == 3 and sub.isdigit()):
            # region subtag
            normalized.append(sub.upper())
        else:
            # extended lang code (3 letters) or Variants (5-8 chars / 4 chars starting with digit)
            normalized.append(sub.lower())

    return "-".join(normalized)

def main():
    tags = ["EN-ca", "ZH-hans-CN", "es-419", "de-CH-1996", "en-US-x-twain"]
    for t in tags:
        print(f"{t:15} -> {bcp47_normalize(t)}")
    return 0

if __name__ == '__main__':
    sys.exit(main())
