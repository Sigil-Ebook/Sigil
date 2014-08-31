require 'pl'
local sum = 0.0
local count = 0
local text = [[
    981124001	2.0	18988.4	10047.1	4149.7
    981125001	0.8	19104.0	9970.4	5088.7
    981127003	0.5	19012.5	9946.9	3831.2
]]
for id,magn,x in input.fields(3,' ',text) do
  sum = sum + x
  count = count + 1
end
print('average x coord is ',sum/count)
