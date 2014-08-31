require 'pl'
function testm(x,s)
  test.asserteq(pretty.number(x,'M'),s)
end

testm(123,'123B')
testm(1234,'1.2KiB')
testm(10*1024,'10.0KiB')
testm(1024*1024,'1.0MiB')

function testn(x,s)
  test.asserteq(pretty.number(x,'N',2),s)
end

testn(123,'123')
testn(1234,'1.23K')
testn(10*1024,'10.24K')
testn(1024*1024,'1.05M')
testn(1024*1024*1024,'1.07B')

function testc(x,s)
  test.asserteq(pretty.number(x,'T'),s)
end

testc(123,'123')
testc(1234,'1,234')
testc(12345,'12,345')
testc(123456,'123,456')
testc(1234567,'1,234,567')
testc(12345678,'12,345,678')

