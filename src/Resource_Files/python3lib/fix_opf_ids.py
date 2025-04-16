import sys
import os
from hrefutils import urldecodepart
from unidecode import unidecode
from uuid import uuid4
from opf_id_parser import Opf_Parser

_ID_VALID_FIRST_CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
_ID_VALID_CHARS = _ID_VALID_FIRST_CHAR + "_-.0123456789"

def GetValidId(newval):
    tmpval = ''
    for c in newval:
        if c in _ID_VALID_CHARS:
            tmpval += c
    newval = tmpval
    if not newval:
        newval = str(uuid4())
    c = newval[0:1]
    if not c in _ID_VALID_FIRST_CHAR:
        newval = 'x' + newval
    return newval

def GenerateBaseIdFromFilename(fn):
    fn, ext = os.path.splitext(fn)
    if ext:
        ext = ext.replace('.','_')
        fn = fn + ext
    fn = fn.strip()
    fn = unidecode(fn)
    return fn

def GenerateUniqueId(val, used_ids):
    baseval = val
    cnt = 0
    while val in used_ids:
        cnt += 1
        val = baseval + ("{0:04d}".format(cnt))
        if cnt >= 10000:
            val = "x" + str(uuid4())
    return val


def rebase_manifest_ids(opfdata):
    op = Opf_Parser(opfdata)

    # get complete list of used ids to use to verify uniqueness
    used_ids = op.get_used_ids()
    
    # map old ids to new ids
    changed_ids = {}

    # walk the manifest creating new ids where needed from current file names
    nmanlist = []
    manlist = op.get_manifest()
    for (id, href, mtype, kvlist) in manlist:
        # base id on base filename_ext asciified and made valid and unqiue
        fn = os.path.basename(urldecodepart(href))
        new_id = GenerateBaseIdFromFilename(fn)
        new_id = GetValidId(new_id)
        if new_id != id:
            if new_id in used_ids:
                new_id = GenerateUniqueId(new_id, used_ids)
            changed_ids[id] = new_id
            if id not in changed_ids.values():
                del used_ids[id]
            used_ids[new_id] = 1
            # print(id, href, id, new_id)
        nmanlist.append((new_id, href, mtype, kvlist))
    manlist = nmanlist

    # walk the spine attribute looking to update the toc if present
    nspineatts = []
    spineatts = op.get_spine_attr()
    for (k, v) in spineatts:
        if k == 'toc':
            v = changed_ids.get(v, v)
        nspineatts.append((k,v))
    spineatts = nspineatts

    # walk the spine entries looking to update the idrefs as needed
    nspentries = []
    spentries = op.get_spine()
    for (idref, kvlist) in spentries:
        idref = changed_ids.get(idref, idref)
        nspentries.append((idref, kvlist))
    spentries = nspentries

    # walk the bindings entries if any exist
    nbindings = []
    bindings = op.get_bindings()
    for (mtype, handler) in bindings:
        handler = changed_ids.get(handler, handler)
        nbindings.append((mtype, handler))
    bindings = nbindings

    # walk the metadata and update cover id as needed
    # and update any meta property refines that pointed into the manifest
    nmdentries = []
    mdentries = op.get_metadata()
    for (mname, mcontent, kvlist) in mdentries:
        if (mname == 'meta'):
            cover_meta = False
            has_property = False
            uses_refines = False
            # walk the attributes of this meta tag
            for (k,v) in kvlist:
                if k == 'name' and v == 'cover':
                    cover_meta=True;
                if k == 'property':
                    has_property = True
                if k == 'refines':
                    uses_refines = True
            if cover_meta or (has_property and uses_refines):
                nkvlist = []
                for (k,v) in kvlist:
                    if cover_meta and k == 'content':
                        v = changed_ids.get(v,v)
                    elif has_property and uses_refines and k == 'refines':
                        if v.startswith('#'):
                            rid = v[1:]
                            v = '#' + changed_ids.get(rid, rid)
                    nkvlist.append((k,v))
                kvlist = nkvlist
        nmdentries.append((mname, mcontent, kvlist))
    mdentries = nmdentries

    # walk the manifest for the second time updating any media-overlay or fallback values
    nmanlist = []
    for (id, href, mtype, kvlist) in manlist:
        nkvlist = []
        for (k, v) in kvlist:
            if k == 'media-overlay':
                v = changed_ids.get(v, v)
            if k == 'fallback':
                v = changed_ids.get(v,v)
            nkvlist.append((k,v))
        kvlist = nkvlist
        nmanlist.append((id, href, mtype, kvlist))
    manlist = nmanlist

    # we are done updating ids so pass the updated data back and recreate the opf
    op.set_metadata(mdentries)
    op.set_manifest(manlist)
    op.set_spine_attr(spineatts)
    op.set_spine(spentries)
    op.set_bindings(bindings)
    ndata = op.rebuild_opfxml()
    return ndata


def main():
    argv = sys.argv
    if len(argv) < 2:
        sys.exit(0)

    if not os.path.exists(argv[1]):
        sys.exit(0)

    with open(argv[1], 'rb') as f:
        data = f.read()
        data = data.decode('utf-8')

    ndata = rebase_manifest_ids(data)
    print(ndata)
    return 0

if __name__ == '__main__':
    sys.exit(main())


