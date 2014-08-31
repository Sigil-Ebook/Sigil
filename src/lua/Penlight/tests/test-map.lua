-- testing Map functionality

require 'pl'

local asserteq = test.asserteq

local cmp = tablex.compare_no_order

local m = Map{alpha=1,beta=2,gamma=3}

assert (cmp(m:values(),{1,2,3}))

assert (cmp(m:keys(),{'alpha','beta','gamma'}))

asserteq (m:items(),{{'alpha',1},{'beta',2},{'gamma',3}})

asserteq (m:getvalues {'alpha','gamma'}, {1,3})
