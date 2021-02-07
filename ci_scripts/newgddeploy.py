#!/usr/bin/env python3

import os
import sys
import subprocess
import datetime
import shutil
import glob

gparent = os.path.expandvars('$GDRIVE_DIR')
grefresh_token = os.path.expandvars('$GDRIVE_REFRESH_TOKEN')

branch = os.path.expandvars('$GITHUB_REF')
commit = os.path.expandvars('$GITHUB_SHA')
build_number = os.path.expandvars('$GITHUB_RUN_NUMBER')

if sys.platform.lower().startswith('darwin'):
    origfilename = './bin/Sigil.tar.xz'
    newfilename = './bin/Sigil-{}-{}-build_num-{}.tar.xz'.format(branch.split('/')[-1], commit[:7], build_number)
else:
    names = glob.glob('.\\installer\\Sigil-*-Setup.exe')
    if not names:
        exit(1)
    origfilename = names[0]
    newfilename = '.\\installer\\Sigil-{}-{}-build_num-{}-Setup.exe'.format(branch.split('/')[-1], commit[:7], build_number)

shutil.copy2(origfilename, newfilename)

folder_name = datetime.date.today()
list_command = ['gdrive',
          '--refresh-token',
          '{}'.format(grefresh_token),
          'list',
          '--no-header',
          '--query',
          'trashed = false and mimeType = \'application/vnd.google-apps.folder\' and \'{}\' in parents and name = \'{}\''.format(gparent, folder_name),
         ]
list_proc = subprocess.run(list_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

if list_proc.returncode == 0 and len(list_proc.stdout):
    gparent = list_proc.stdout.split()[0]
else:
    mk_command = ['gdrive',
                  '--refresh-token',
                  '{}'.format(grefresh_token),
                  'mkdir',
                  '--parent',
                  '{}'.format(gparent),
                  '{}'.format(folder_name),
                  ]
    mk_proc = subprocess.run(mk_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

    if mk_proc.returncode == 0 and mk_proc.stdout.strip():
        print('Created new \'{}\' folder'.format(folder_name))
        gparent = mk_proc.stdout.split()[1]

up_command = ['gdrive',
              '--refresh-token',
              '{}'.format(grefresh_token),
              'upload',
              '--parent',
              '{}'.format(gparent),
              '{}'.format(newfilename)
              ]
up_proc = subprocess.run(up_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

info = None
if up_proc.returncode == 0:
    print('Uploaded {} to \'{}\' folder'.format(newfilename, folder_name))
    info = up_proc.stdout.splitlines()[1].split()[1]

if info is not None:
    inf_command = ['gdrive',
                   '--refresh-token',
                  '{}'.format(grefresh_token),
                  'info',
                  '{}'.format(info),
                   ]
    inf_proc = subprocess.run(inf_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    if inf_proc.returncode == 0:
        inf_dic = {k.split(':')[0].strip():k.split(': ')[1] for k in inf_proc.stdout.splitlines()}
        print('Download {} from {}'.format(newfilename, inf_dic["DownloadUrl"]))
    os.remove(newfilename)
else:
    os.remove(newfilename)
    exit(1)

