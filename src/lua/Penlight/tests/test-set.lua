class = require 'pl.class'
M = require 'pl.Map'
S = require 'pl.Set'
List = require 'pl.List'

asserteq = require 'pl.test' . asserteq
asserteq2 = require 'pl.test' . asserteq2
MultiMap = require 'pl.MultiMap'
OrderedMap = require 'pl.OrderedMap'

s1 = S{1,2}
s2 = S{1,2}
-- equality
asserteq(s1,s2)
-- union
asserteq(S{1,2} + S{2,3},S{1,2,3})
-- intersection
asserteq(S{1,2} * S{2,3}, S{2})
-- symmetric_difference
asserteq(S{1,2} ^ S{2,3}, S{1,3})
-- tostring - illustrative, because these assertions may or may not work,
-- due to no ordering in set elements
--asserteq(tostring(S{1,2}),'[1,2]')
--asserteq(tostring(S{1,S{2,3}}),'[1,[2,3]]')

s3 = S()
asserteq(S.isempty(s3),true)

s4 = S{1,2,3}

-- subsets/supersets
asserteq(s4 > s1,true)

-- union and intersection
asserteq(S{1,2}+S{2,3},S{1,2,3})
asserteq(S{1,2}*S{2,3},S{2})

S.set(s3,'one',true)
s3.two = true
asserteq(s3,S{'one','two'})

m = M{one=1,two=2}
asserteq(m,M{one=1,two=2})
m:update {three=3,four=4}
asserteq(m,M{one=1,two=2,three=3,four=4})

class.Animal()

function Animal:_init(name)
    self.name = name
end

function Animal:__tostring()
  return self.name..': '..self:speak()
end

class.Dog(Animal)

function Dog:speak()
  return 'bark'
end

class.Cat(Animal)

function Cat:_init(name,breed)
    self:super(name)  -- must init base!
    self.breed = breed
end

function Cat:speak()
  return 'meow'
end

Lion = class(Cat)

function Lion:speak()
  return 'roar'
end

fido = Dog('Fido')
felix = Cat('Felix','Tabby')
leo = Lion('Leo','African')

asserteq(tostring(fido),'Fido: bark')
asserteq(tostring(felix),'Felix: meow')
asserteq(tostring(leo),'Leo: roar')

assert(Dog:class_of(fido))
assert(fido:is_a(Dog))

assert(leo:is_a(Animal))

m = MultiMap()
m:set('john',1)
m:set('jane',3)
m:set('john',2)

ms = MultiMap{john={1,2},jane={3}}

asserteq(m,ms)

m = OrderedMap()
m:set('one',1)
m:set('two',2)
m:set('three',3)

asserteq(m:values(),List{1,2,3})

-- usually exercized like this:
--for k,v in m:iter() do print(k,v) end

fn = m:iter()
asserteq2 ('one',1,fn())
asserteq2 ('two',2,fn())
asserteq2 ('three',3,fn())

o1 = OrderedMap  {{z=2},{beta=1},{name='fred'}}
asserteq(tostring(o1),'{z=2,beta=1,name="fred"}')

-- order of keys is not preserved here!
o2 = OrderedMap   {z=4,beta=1.1,name='alice',extra='dolly'}

o1:update(o2)
asserteq(tostring(o1),'{z=4,beta=1.1,name="alice",extra="dolly"}')

o1:set('beta',nil)
asserteq(o1,OrderedMap{{z=4},{name='alice'},{extra='dolly'}})

o3 = OrderedMap()
o3:set('dog',10)
o3:set('cat',20)
o3:set('mouse',30)

asserteq(o3:keys(),{'dog','cat','mouse'})

o3:set('dog',nil)

asserteq(o3:keys(),{'cat','mouse'})

-- Vadim found a problem when clearing a key which did not exist already.
-- The keys list would then contain the key, although the map would not
o3:set('lizard',nil)

asserteq(o3:keys(),{'cat','mouse'})
asserteq(o3:values(), {20,30})
asserteq(tostring(o3),'{cat=20,mouse=30}')

-- copy constructor
o4 = OrderedMap(o3)

asserteq(o4,o3)

-- constructor throws an error if the argument is bad
-- (errors same as OrderedMap:update)
asserteq(false,pcall(function()
    m = OrderedMap('string')
end))

---- changing order of key/value pairs ----

o3 = OrderedMap{{cat=20},{mouse=30}}

o3:insert(1,'bird',5) -- adds key/value before specified position
o3:insert(1,'mouse') -- moves key keeping old value
asserteq(o3:keys(),{'mouse','bird','cat'})
asserteq(tostring(o3),'{mouse=30,bird=5,cat=20}')
o3:insert(2,'cat',21) -- moves key and sets new value
asserteq(tostring(o3),'{mouse=30,cat=21,bird=5}')
-- if you don't specify a value for an unknown key, nothing happens to the map
o3:insert(3,'alligator')
asserteq(tostring(o3),'{mouse=30,cat=21,bird=5}')

---- short-cut notation

o5 = OrderedMap()
o5.alpha = 1
o5.beta = 2
o5.gamma = 3

asserteq(o5,OrderedMap{{alpha=1},{beta=2},{gamma=3}})

o5.alpha = 10
o5.beta = 20
o5.gamma = 30
o5.delta = 40
o5.checked = false

asserteq(o5,OrderedMap{{alpha=10},{beta=20},{gamma=30},{delta=40},{checked=false}})





