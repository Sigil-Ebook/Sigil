-- very simple lexer program which looks at all identifiers in a Lua
-- file and checks whether they're in the global namespace.
-- At the end, we dump out the result of count_map, which will give us
-- unique identifiers with their usage count.
-- (an example of a program which itself needs to be careful about what
-- goes into the global namespace)

local utils = require 'pl.utils'
local file = require 'pl.file'
local lexer = require 'pl.lexer'
local List = require 'pl.List'
local pretty = require 'pl.pretty'
local seq = require 'pl.seq'

utils.on_error 'quit'

local txt,err = file.read(arg[1] or 'testglobal.lua')
local globals = List()
for t,v in lexer.lua(txt) do
	if t == 'iden' and rawget(_G,v) then
		globals:append(v)
	end
end

pretty.dump(seq.count_map(globals))


