--cloning a directory tree.

require 'pl'
p1 = [[examples]]
p2 = [[copy/of/examples]]

if not path.isfile 'examples/testclone.lua' then
	return print 'please run this in the penlight folder (below examples)'
end

-- make a copy of the examples folder
dir.clonetree(p1,p2,dir.copyfile)

assert(path.isdir 'copy')

print '---'
t = os.time()
print(lfs.touch('examples/testclone.lua',t,t+10))

-- this should only update this file
dir.clonetree(p1,p2,
function(f1,f2)
  local t1 = path.getmtime(f1)
  local t2 = path.getmtime(f2)
  --print(f1,t1,f2,t2)
  if t1 > t2 then
	dir.copyfile(f1,f2)
	print(f1,f2,t1,t2)
  end
  return true
end)

-- and get rid of the whole copy directory, with subdirs
dir.rmtree 'copy'

assert(not path.exists 'copy')


