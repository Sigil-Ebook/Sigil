-- an example showing 'pl.lexer' doing some serious work.
-- The resulting Lua table is in the same LOM format used by luaexpat.
-- This is (clearly) not a professional XML parser, so don't use it
-- on your homework!

require 'pl'

local append = table.insert
local skipws,expecting = lexer.skipws,lexer.expecting

function parse_element (tok,tag)
	local tbl,t,v,attrib
	tbl = {}
	tbl.tag = tag  -- LOM 'tag' is the element tag
	t,v = skipws(tok)
	while v ~= '/' and v ~= '>' do
		if t ~= 'iden' then error('expecting attribute identifier') end
		attrib = v
		expecting(tok,'=')
		v = expecting(tok,'string')
		-- LOM: 'attr' subtable contains attrib/value pairs and an ordered list of attribs
		if not tbl.attr then tbl.attr = {} end
		tbl.attr[attrib] = v
		append(tbl.attr,attrib)
		t,v = skipws(tok)
	end
	if v == '/' then
		expecting(tok,'>')
		return tbl
	end
	-- pick up element data
	t,v = tok()
	while true do
		if t == '<' then
			t,v = skipws(tok)
			if t == '/' then -- element end tag
				t,v = tok()
				if t == '>' then return tbl end
				if t == 'iden' and v == tag then
					if tok() == '>' then return tbl end
				end
				error('expecting end tag '..tag)
			else
				append(tbl,parse_element(tok,v)) -- LOM: child elements added to table
				t,v = skipws(tok)
			end
		else
			append(tbl,v) -- LOM: text added to table
			t,v = skipws(tok)
		end
	end
end

function parse_xml (tok)
    local t,v = skipws(tok)
	while t == '<' do
		t,v = tok()
		if t == '?' or t == '!' then
			-- skip meta stuff and commentary
			repeat t = tok() until t == '>'
			t,v = expecting(tok,'<')
		else
			return parse_element(tok,v)
		end
	end
end

s = [[
<?xml version="1.0" encoding="UTF-8"?>
<sensor name="closure-meter-2" id="7D7D0600006F0D00" loc="100,100,0" device="closure-meter" init="true">
<detector name="closure-meter" phenomenon="closure" units="mm" id="1"
    vmin="0" vmax="5000" device="closure-meter" calib="0,0;5000,5000"
    sampling_interval="25000" measurement_interval="600000"
/>
</sensor>
]]

local tok = lexer.scan(s,nil,{space=false},{string=true})
local res = parse_xml(tok)
print(pretty.write(res))

