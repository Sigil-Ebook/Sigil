#!/usr/bin/env python3
from __future__ import unicode_literals, division, absolute_import, print_function

import sys
import os
import re

def check_for_updates(site_url):
    latest_version = ""
    _version_pattern = re.compile(r'<current-version>([^<]*)</current-version>')

    try:
        import socket
        from urllib.request import Request, urlopen
        socket.setdefaulttimeout(3)

        # get the latest version from the plugin release website page
        req = Request(site_url)
        response = urlopen(req)
        the_page = response.read()
        the_page = the_page.decode('utf-8')
        m = _version_pattern.search(the_page)
        if m:
            latest_version = (m.group(1).strip())
    except:
        pass

    return latest_version

