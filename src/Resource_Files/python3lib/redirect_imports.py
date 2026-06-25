import sys
import importlib.util

# Force sys.modules to point 're' to the 'regex' library if it's present.
# If regex is not available, re is used.
if importlib.util.find_spec("regex") is not None:
    import regex
    sys.modules['re'] = regex
    print("Sigil's embedded python is substituting the regex module for re")
