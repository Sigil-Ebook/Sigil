#!/usr/bin/env python3
from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os
import re
import socket

def is_connected():
    try:
        # An ugly little hack that will QUICKLY tell us whether
        # or not there's an active internet connection. IP only
        # to avoid any name-resolving delays. Just a quick check
        # to see if one of Google's primary nameservers is available.
        # Otherwise Sigil's launch could be ridiculously postponed
        # by an ssl timeout not being honored.

        # I'd rather an update check fail than Sigil's launch be
        # needlessly delayed.
        sock = socket.create_connection(('8.8.8.8', 53), 1)
        sock.close()
        return True
    except:
        pass

    return False

def check_for_updates(site_url):
    latest_version = ""
    _version_pattern = re.compile(r'<current-version>([^<]*)</current-version>')

    if is_connected():
        try:
            from urllib.request import Request, urlopen
            socket.setdefaulttimeout(2)

            # get the latest version from the plugin release website page
            req = Request(site_url)
            response = urlopen(req, timeout=2)
            the_page = response.read()
            the_page = the_page.decode('utf-8')
            m = _version_pattern.search(the_page)
            if m:
                latest_version = (m.group(1).strip())
        except:
            pass

    return latest_version

