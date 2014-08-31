## Date and Time

<a id="date"></a>

### Creating and Displaying Dates

The `Date` class provides a simplified way to work with [date and
time](http://www.lua.org/pil/22.1.html) in Lua; it leans heavily on the functions
`os.date` and `os.time`.

A `Date` object can be constructed from a table, just like with `os.time`.
Methods are provided to get and set the various parts of the date.

    > d = Date {year = 2011, month = 3, day = 2 }
    > = d
    2011-03-02 12:00:00
    > = d:month(),d:year(),d:day()
    3	2011	2
    > d:month(4)
    > = d
    2011-04-02 12:00:00
    > d:add {day=1}
    > = d
    2011-04-03 12:00:00

`add` takes a table containing one of the date table fields.

    > = d:weekday_name()
    Sun
    > = d:last_day()
    2011-04-30 12:00:00
    > = d:month_name(true)
    April

There is a default conversion to text for date objects, but `Date.Format` gives
you full control of the format for both parsing and displaying dates:

    > iso = Date.Format 'yyyy-mm-dd'
    > d = iso:parse '2010-04-10'
    > amer = Date.Format 'mm/dd/yyyy'
    > = amer:tostring(d)
    04/10/2010

With the 0.9.7 relase, the `Date` constructor has become more flexible. You may
omit any of the 'year', 'month' or 'day' fields:

    > = Date { year = 2008 }
    2008-01-01 12:00:00
    > = Date { month = 3 }
    2011-03-01 12:00:00
    > = Date { day = 20 }
    2011-10-20 12:00:00
    > = Date { hour = 14, min = 30 }
    2011-10-13 14:30:00

If 'year' is omitted, then the current year is assumed, and likewise for 'month'.

To set the time on such a partial date, you can use the fact that the 'setter'
methods return the date object and so you can 'chain' these methods.

    > d = Date { day = 03 }
    > = d:hour(18):min(30)
    2011-10-03 18:30:00

Finally, `Date` also now accepts positional arguments:

    > = Date(2011,10,3)
    2011-10-03 12:00:00
    > = Date(2011,10,3,18,30,23)
    2011-10-03 18:30:23

`Date.format` has been extended. If you construct an instance without a pattern,
then it will try to match against a set of known formats. This is useful for
human-input dates since keeping to a strict format is not one of the strong
points of users. It assumes that there will be a date, and then a date.

    > df = Date.Format()
    > = df:parse '5.30pm'
    2011-10-13 17:30:00
    > = df:parse '1730'
    nil     day out of range: 1730 is not between 1 and 31
    > = df:parse '17.30'
    2011-10-13 17:30:00
    > = df:parse 'mar'
    2011-03-01 12:00:00
    > = df:parse '3 March'
    2011-03-03 12:00:00
    > = df:parse '15 March'
    2011-03-15 12:00:00
    > = df:parse '15 March 2008'
    2008-03-15 12:00:00
    > = df:parse '15 March 2008 1.30pm'
    2008-03-15 13:30:00
    > = df:parse '2008-10-03 15:30:23'
    2008-10-03 15:30:23

ISO date format is of course a good idea if you need to deal with users from
different countries. Here is the default behaviour for 'short' dates:

    > = df:parse '24/02/12'
    2012-02-24 12:00:00

That's not what Americans expect! It's tricky to work out in a cross-platform way
exactly what the expected format is, so there is an explicit flag:

    > df:US_order(true)
    > = df:parse '9/11/01'
    2001-11-09 12:00:00

