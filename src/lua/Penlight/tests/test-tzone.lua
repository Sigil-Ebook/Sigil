require 'pl'
local df = Date.Format()
local dl = df:parse '2008-07-05'
local du = dl:toUTC()

test.asserteq(dl,du)



