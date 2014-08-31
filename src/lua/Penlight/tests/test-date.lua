local test = require 'pl.test'
local app = require 'pl.app'
local utils = require 'pl.utils'
local asserteq, assertmatch = test.asserteq, test.assertmatch
local dump = require 'pl.pretty'.dump
local T = require 'pl.test'.tuple

local Date = require 'pl.Date'

--[[
d = Date()
print(d)
print(d:year())
d:day(20)
print(d)
d:add {day = 2}
print(d:day())
d = Date() -- 'now'
print(d:last_day():day())
print(d:month(7):last_day())
--]]

function check_df(fmt,str,no_check)
    local df = Date.Format(fmt)
    local d = df:parse(str)
    --print(str,d)
    if not no_check then
        asserteq(df:tostring(d),str)
    end
end

check_df('dd/mm/yy','02/04/10')
check_df('mm/dd/yyyy','04/02/2010')
check_df('yyyy-mm-dd','2011-02-20')
check_df('yyyymmdd','20070320')

-- use single fields for 'slack' parsing
check_df('m/d/yyyy','1/5/2001',true)

check_df('HH:MM','23:10')

iso = Date.Format 'yyyy-mm-dd' -- ISO date
d = iso:parse '2010-04-10'
asserteq(T(d:day(),d:month(),d:year()),T(10,4,2010))
amer = Date.Format 'mm/dd/yyyy' -- American style
s = amer:tostring(d)
dc = amer:parse(s)
asserteq(d,dc)

d = Date() -- today
d:add { day = 1 }  -- tomorrow
assert(d > Date())

--------- Time intervals -----
-- new explicit Date.Interval class; also returned by Date:diff
d1 = Date.Interval(1202)
d2 = Date.Interval(1500)
asserteq(tostring(d2:diff(d1)),"4 min 58 sec ")

-------- testing 'flexible' date parsing ---------


local df = Date.Format()

function parse_date (s)
    return df:parse(s)
end

-- ISO 8601
-- specified as UTC plus/minus offset

function parse_utc (s)
    local d = parse_date(s)    
    return d:toUTC()
end

asserteq(parse_utc '2010-05-10 12:35:23Z', Date(2010,05,10,12,35,23))
asserteq(parse_utc '2008-10-03T14:30+02', Date(2008,10,03,12,30))
asserteq(parse_utc '2008-10-03T14:00-02:00',Date(2008,10,03,16,0))

---- can't do anything before 1970, which is somewhat unfortunate....
--parse_date '20/03/59'

asserteq(parse_date '15:30', Date {hour=15,min=30})
asserteq(parse_date '8.05pm', Date {hour=20,min=5})
asserteq(parse_date '28/10/02', Date {year=2002,month=10,day=28})
asserteq(parse_date ' 5 Feb 2012 ', Date {year=2012,month=2,day=5})
asserteq(parse_date '20 Jul ', Date {month=7,day=20})
asserteq(parse_date '05/04/02 15:30:43', Date{year=2002,month=4,day=5,hour=15,min=30,sec=43})
asserteq(parse_date 'march', Date {month=3})
asserteq(parse_date '2010-05-23T0130', Date{year=2010,month=5,day=23,hour=1,min=30})
asserteq(parse_date '2008-10-03T14:30:45', Date{year=2008,month=10,day=3,hour=14,min=30,sec=45})

function err (status,e)
    return e
end

assertmatch(err(parse_date('2005-10-40 01:30')),'40 is not between 1 and 31')
assertmatch(err(parse_date('14.20pm')),'14 is not between 0 and 12')

local d = parse_date '2007-08-10'
-- '+' works like add, but can also work with intervals
local nxt = d + {month=1}
-- '-' is an alias for diff method
asserteq(tostring(nxt - d), '1 month ')

--- Can explicitly get UTC date; these of course refer to same time
local now,utc  = Date(), Date 'utc'
asserteq(tostring(now - utc),'zero')

if app.platform() ~= 'Windows' then
    if not utils.execute ("TZ='Europe/London' "..app.lua().." test-tzone.lua") then
        error "buggered!"
    end
end
