sip = require 'pl.sip'
tablex = require 'pl.tablex'
utils = require 'pl.utils'

local function dump(t)
	if not t or type(t) ~= 'table' then print '<nada>'; return end
	for k,v in pairs(t) do
		print(k,v,type(v))
	end
end

function check(pat,line,tbl)
    local parms = {}
	if type(pat) == 'string' then
		pat = sip.compile(pat)
	end
    local res = pat(line,parms)
    if res then
		if not tablex.deepcompare(parms,tbl) then
			print 'parms'
			dump(parms)
			print 'tbl'
			dump(tbl)
			utils.quit(1,'failed!')
		end
    else -- only should happen if we're passed a nil!
        assert(tbl == nil)
    end
end

c = sip.compile('ref=$S{file}:$d{line}')
check(c,'ref=bonzo:23',{file='bonzo',line=23})
check(c,'here we go ref=c:\\bonzo\\dog.txt:53',{file='c:\\bonzo\\dog.txt',line=53})
check(c,'here is a line ref=xxxx:xx',nil)

c = sip.compile('($i{x},$i{y},$i{z})')
check(c,'(10,20,30)',{x=10,y=20,z=30})
check(c,'  (+233,+99,-40) ',{x=233,y=99,z=-40})

local pat = '$v{name} = $q{str}'
--assert(sip.create_pattern(pat) == [[([%a_][%w_]*)%s*=%s*(["'])(.-)%2]])
local m = sip.compile(pat)

check(m,'a = "hello"',{name='a',str='hello'})
check(m,"a = 'hello'",{name='a',str='hello'})
check(m,'_fred="some text"',{name='_fred',str='some text'})

-- some cases broken in 0.6b release
check('$v is $v','bonzo is dog for sure',{'bonzo','dog'})
check('$v is $','bonzo is dog for sure',{'bonzo','dog for sure'})
check('$v $d','age 23',{'age',23})


months={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"}

function adjust_and_check(res)
	if res.year < 100 then
		if res.year < 70 then
			res.year = res.year + 2000
		else
			res.year = res.year + 1900
		end
	end
end

shortdate = sip.compile('$d{day}/$d{month}/$d{year}')
longdate = sip.compile('$d{day} $v{mon} $d{year}')
isodate = sip.compile('$d{year}-$d{month}-$d{day}')

function dcheck (d1,d2)
    adjust_and_check(d1)
    assert(d1.day == d2.day and d1.month == d2.month and d1.year == d2.year)
end

function dates(str,tbl)
	local res = {}
	if shortdate(str,res) then
		dcheck(res,tbl)
    elseif isodate(str,res) then
        dcheck(res,tbl)
	elseif longdate(str,res) then
		res.month = tablex.find(months,res.mon)
		dcheck(res,tbl)
	else
		assert(tbl == nil)
	end
end

dates ('10/12/2007',{year=2007,month=12,day=10})
dates ('2006-03-01',{year=2006,month=3,day=1})
dates ('25/07/05',{year=2005,month=7,day=25})
dates ('20 Mar 1959',{year=1959,month=3,day=20})







