from __future__ import unicode_literals, division, absolute_import, print_function
from . import errorhandler
import css_parser.encutils as encutils
from css_parser.version import VERSION
"""Default URL reading functions"""


__all__ = ['_defaultFetcher']
__docformat__ = 'restructuredtext'
__version__ = '$Id: tokenize2.py 1547 2008-12-10 20:42:26Z cthedot $'

import sys

if sys.version_info[0] >= 3:
    from urllib.request import urlopen as urllib_urlopen
    from urllib.request import Request as urllib_Request
    from urllib.error import HTTPError as urllib_HTTPError
    from urllib.error import URLError as urllib_URLError
else:
    from urllib2 import urlopen as urllib_urlopen
    from urllib2 import Request as urllib_Request
    from urllib2 import HTTPError as urllib_HTTPError
    from urllib2 import URLError as urllib_URLError


log = errorhandler.ErrorHandler()


def _defaultFetcher(url):
    """Retrieve data from ``url``. css_parser default implementation of fetch
    URL function.

    Returns ``(encoding, string)`` or ``None``
    """
    try:
        request = urllib_Request(url)
        request.add_header('User-agent',
                           'css_parser %s (http://www.cthedot.de/css_parser/)' % VERSION)
        res = urllib_urlopen(request)
    except urllib_HTTPError as e:
        # http error, e.g. 404, e can be raised
        log.warn('HTTPError opening url=%s: %s %s' %
                 (url, e.code, e.msg), error=e)
    except urllib_URLError as e:
        # URLError like mailto: or other IO errors, e can be raised
        log.warn('URLError, %s' % e.reason, error=e)
    except OSError as e:
        # e.g if file URL and not found
        log.warn(e, error=OSError)
    except ValueError as e:
        # invalid url, e.g. "1"
        log.warn('ValueError, %s' % e.args[0], error=ValueError)
    else:
        if res:
            mimeType, encoding = encutils.getHTTPInfo(res)
            if mimeType != 'text/css':
                log.error('Expected "text/css" mime type for url=%r but found: %r' %
                          (url, mimeType), error=ValueError)
            content = res.read()
            if hasattr(res, 'close'):
                res.close()
            return encoding, content
