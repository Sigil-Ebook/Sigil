import re
import os

from fr_utils import unescapeit, toUpper, toLower, swapcase, capitalize

from titlecase import titlecase

def replace_entities(text):
    return unescapeit(text)

def handle_entities(text, func):
    return func(replace_entities(text))

# the following functions wre all borrow from calibre and modified to work without icu
# and to work in the Sigil plugin context.

def apply_func_to_match_groups(match, func=toUpper, handle_entities=handle_entities):
    '''Apply the specified function to individual groups in the match object (the result of re.search() or
    the whole match if no groups were defined. Returns the replaced string.'''

    parts, pos = [], match.start()
    def f(text):
        return handle_entities(text, func)

    n = len(match.groups())
    if n > 0:
        ngroups = n + 1
        i = 1
        while i < ngroups:
            start, end = match.span(i)
            print(start, end)
            if start > -1:
                parts.append(match.string[pos:start])
                parts.append(f(match.string[start:end]))
                pos = end
            i = i + 1
        parts.append(match.string[pos:match.end()])
        return ''.join(parts)
    return f(match.group())

def apply_func_to_html_text(match, func=toUpper, handle_entities=handle_entities):
    ''' Apply the specified function only to text between HTML tag definitions. '''
    def f(text):
        return handle_entities(text, func)
    parts = re.split(r'(<[^>]+>)', match.group())
    parts = (x if x.startswith('<') else f(x) for x in parts)
    return ''.join(parts)


# ('Upper-case text', upper, apply_func_to_match_groups)
def replace_uppercase(match, number, file_name, metadata, data, *args, **kwargs):
    '''Make matched text upper case. If the regular expression contains groups,
    only the text in the groups will be changed, otherwise the entire text is
    changed.'''
    return apply_func_to_match_groups(match, toUpper)


# ('Lower-case text', lower, apply_func_to_match_groups)
def replace_lowercase(match, number, file_name, metadata, data, *args, **kwargs):
    '''Make matched text lower case. If the regular expression contains groups,
    only the text in the groups will be changed, otherwise the entire text is
    changed.'''
    return apply_func_to_match_groups(match, toLower)


# ('Capitalize text', capitalize, apply_func_to_match_groups)
def replace_capitalize(match, number, file_name, metadata, data, *args, **kwargs):
    '''Capitalize matched text. If the regular expression contains groups,
    only the text in the groups will be changed, otherwise the entire text is
    changed.'''
    return apply_func_to_match_groups(match, capitalize)


# ('Title-case text', titlecase, apply_func_to_match_groups)
def replace_titlecase(match, number, file_name, metadata, data):
    '''Title-case matched text. If the regular expression contains groups,
    only the text in the groups will be changed, otherwise the entire text is
    changed.'''
    return apply_func_to_match_groups(match, titlecase)


#('Swap the case of text', swapcase, apply_func_to_match_groups)
def replace_swapcase(match, number, file_name, metadata, data, *args, **kwargs):
    '''Swap the case of the matched text. If the regular expression contains groups,
    only the text in the groups will be changed, otherwise the entire text is
    changed.'''
    return apply_func_to_match_groups(match, swapcase)


# ('Upper-case text (ignore tags)', upper, apply_func_to_html_text)
def replace_uppercase_ignore_tags(match, number, file_name, metadata, data, *args, **kwargs):
    '''Make matched text upper case, ignoring the text inside tag definitions.'''
    return apply_func_to_html_text(match, toUpper)


# ('Lower-case text (ignore tags)', lower, apply_func_to_html_text)
def replace_lowercase_ignore_tags(match, number, file_name, metadata, data, *args, **kwargs):
    '''Make matched text lower case, ignoring the text inside tag definitions.'''
    return apply_func_to_html_text(match, toLower)


#('Capitalize text (ignore tags)', capitalize, apply_func_to_html_text)
def replace_capitalize_ignore_tags(match, number, file_name, metadata, data, *args, **kwargs):
    '''Capitalize matched text, ignoring the text inside tag definitions.'''
    return apply_func_to_html_text(match, capitalize)


#('Title-case text (ignore tags)', titlecase, apply_func_to_html_text)
def replace_titlecase_ignore_tags(match, number, file_name, metadata, data, *args, **kwargs):
    '''Title-case matched text, ignoring the text inside tag definitions.'''
    return apply_func_to_html_text(match, titlecase)


# ('Swap the case of text (ignore tags)', swapcase, apply_func_to_html_text)
def replace_swapcase_ignore_tags(match, number, file_name, metadata, data, *args, **kwargs):
    '''Swap the case of the matched text, ignoring the text inside tag definitions.'''
    return apply_func_to_html_text(match, swapcase)


def replace_debug_log(message):
    logfile = os.environ.get('SIGIL_FUNCTION_REPLACE_LOGFILE', None);
    if logfile:
        with open(logfile, "a", encoding="utf-8") as f:
            f.write(message)
