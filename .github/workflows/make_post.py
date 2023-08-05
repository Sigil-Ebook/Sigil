import os
import sys
import getopt
import json
import codecs
from dateutil import parser


BAIL = False

NAME = 'Sigil'

# Blog post template
POST = """---
title: {}
date: {}
categories:
  - Blog
tags:
  - Releases
  - {}
---

{}

{}

"""

# Map asset filename patterns to link text
asset_patterns = {
    'CHECKSUMS.sha256.txt'     : 'CHECKSUMS file',
    'Windows-Setup.exe'        : 'Windows x86 download',
    'Windows-x64-Setup.exe'    : 'Windows x64 download',
    'Windows-Legacy-Setup.exe' : 'Windows Legacy download',
    'Mac-x86_64.txz'           : 'MacOS (Intel) download',
    'Mac-arm64.txz'            : 'MacOS (Arm64) download'
}

link = "[{}]({}){{: .btn .btn--success}}<br/>"

def rchop(s, sub):
    return s[:-len(sub)] if s.endswith(sub) else s

# Get asset download urls from release  event payload 
def get_asset_links(assets):
    md_links =[]
    for asset in assets:
        for p, t in asset_patterns.items():
            if p in asset['name']:
                md_links.append(link.format(t, asset['browser_download_url']))

    print(md_links)
    return rchop('\n'.join(md_links), '<br/>')


def main(argv):
    global BAIL
    global POST
    path = ''
    try:
        opts, args = getopt.getopt(argv,"hp:",["path="])
    except getopt.GetoptError:
        print ('make_post.py -p <path>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('make_post.py -p <path>')
            sys.exit()
        elif opt in ("-p", "--path"):
            path = arg

    # Open and serialize release event payload
    with codecs.open(path, 'r', 'utf-8') as f:
        j = json.load(f)
    r = j['release']
    date = parser.parse(r['published_at'])
    title = r['name'] + ' Released'

    # file name for blog post
    filename = os.path.join('.', date.strftime('%Y-%m-%d') + '-' + title.lower().replace(' ', '-') + '.md')
    print(filename)

    links = get_asset_links(r['assets'])

    # Plug relevant data into post template
    md = POST.format(title, r['published_at'], NAME, links, r['body'])
    print(md)

    # Write post to file
    with codecs.open(filename, 'w', 'utf-8') as f:
        f.write(md)
    sys.exit()

if __name__ == "__main__":
   main(sys.argv[1:])
