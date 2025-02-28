import html

def unescapeit(sval):
    return html.unescape(sval)

def toUpper(text):
    return text.upper()

def toLower(text):
    return text.lower()

def swapcase(text):
    result = []
    for c in text:
        if c.islower(): 
            result.append(c.upper())
        elif c.isupper():
            result.append(c.lower())
        else:
            result.append(c)
    return ''.join(result)
 
def capitalize(x):
    try:
        return toUpper(x[0]) + toLower(x[1:])
    except (IndexError, TypeError, AttributeError):
        return x

