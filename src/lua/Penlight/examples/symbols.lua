require 'pl'
utils.import 'pl.func'
local ops = require 'pl.operator'
local List = require 'pl.List'
local append,concat = table.insert,table.concat
local compare,find_if,compare_no_order,imap,reduce,count_map = tablex.compare,tablex.find_if,tablex.compare_no_order,tablex.imap,tablex.reduce,tablex.count_map
local unpack = utils.unpack

function bindval (self,val)
    rawset(self,'value',val)
end

local optable = ops.optable

function sexpr (e)
	if isPE(e) then
		if e.op ~= 'X' then
			local args = tablex.imap(sexpr,e)
			return '('..e.op..' '..table.concat(args,' ')..')'
		else
			return e.repr
		end
	else
		return tostring(e)
	end
end


psexpr = compose(print,sexpr)



function equals (e1,e2)
    local p1,p2 = isPE(e1),isPE(e2)
    if p1 ~= p2 then return false end  -- different kinds of animals!
    if p1 and p2 then -- both PEs
        -- operators must be the same
        if e1.op ~= e2.op then return false end
        -- PHs are equal if their representations are equal
        if e1.op == 'X' then return e1.repr == e2.repr
        -- commutative operators
        elseif e1.op == '+' or e1.op == '*' then
            return compare_no_order(e1,e2,equals)
        else
            -- arguments must be the same
            return compare(e1,e2,equals)
        end
    else -- fall back on simple equality for non PEs
        return e1 == e2
    end
end

-- run down an unbalanced operator chain (like a+b+c) and return the arguments {a,b,c}
function tcollect (op,e,ls)
    if isPE(e) and e.op == op then
        for i = 1,#e do
            tcollect(op,e[i],ls)
        end
    else
        ls:append(e)
        return
    end
end

function rcollect (e)
    local res = List()
    tcollect(e.op,e,res)
    return res
end


-- balance ensures that +/* chains are collected together, operates in-place.
-- thus (+(+ a b) c) or (+ a (+ b c)) becomes (+ a b c), order immaterial
function balance (e)
    if isPE(e) and e.op ~= 'X' then
        local op,args = e.op
        if op == '+' or op == '*' then
            args = rcollect(e)
        else
            args = imap(balance,e)
        end
        for i = 1,#args do
            e[i] = args[i]
        end
    end
    return e
end

-- fold constants in an expression
function fold (e)
    if isPE(e) then
        if e.op == 'X' then
            -- there could be _bound values_!
            local val = rawget(e,'value')
            return val and val or e
        else
            local op = e.op
            local addmul = op == '*' or op == '+'
            -- first fold all arguments
            local args = imap(fold,e)
            if not addmul and not find_if(args,isPE) then
                -- no placeholders in these args, we can fold the expression.
                local opfn = optable[op]
                if opfn then
                    return opfn(unpack(args))
                else
                    return '?'
                end
            elseif addmul then
                -- enforce a few rules for + and *
                -- split the args into two classes, PE args and non-PE args.
                local classes = List.partition(args,isPE)
                local pe,npe = classes[true],classes[false]
                if npe then -- there's at least one non PE argument
                    -- so fold them
                    if #npe == 1 then npe = npe[1]
                    else npe = npe:reduce(optable[op])
                    end
                    -- if the result is a constant, return it
                    if not pe then return npe end

                    -- either (* 1 x) => x or (* 1 x y ...) => (* x y ...)
                    if op == '*' then
                        if npe == 0 then return 0
                        elseif npe == 1 then -- identity
                            if #pe == 1 then return pe[1] else npe = nil end
                        end
                    else -- special cases for +
                        if npe == 0 then -- identity
                            if #pe == 1 then return pe[1] else npe = nil end
                        end
                    end
                end
                -- build up the final arguments
                local res = {}
                if npe then append(res,npe) end
                for val,count in pairs(count_map(pe,equals)) do
                    if count > 1 then
                        if op == '*' then val = val ^ count
                        else val = val * count
                        end
                    end
                    append(res,val)
                end
                if #res == 1 then return res[1] end
                return PE{op=op,unpack(res)}
            elseif op == '^' then
                if args[2] == 1 then return args[1] end -- identity
                if args[2] == 0 then return 1 end
            end
            return PE{op=op,unpack(args)}
        end
    else
        return e
    end
end

function expand (e)
    if isPE(e) and e.op == '*' and isPE(e[2]) and e[2].op == '+' then
        local a,b = e[1],e[2]
        return expand(b[1]*a) + expand(b[2]*a)
    else
        return e
    end
end

function isnumber (x)
    return type(x) == 'number'
end

-- does this PE contain a reference to x?
function references (e,x)
    if isPE(e) then
        if e.op == 'X' then return x.repr == e.repr
        else
            return find_if(e,references,x)
        end
    else
        return false
    end
end

local function muli (args)
    return PE{op='*',unpack(args)}
end

local function addi (args)
    return PE{op='+',unpack(args)}
end

function diff (e,x)
    if isPE(e) and references(e,x) then
        local op = e.op
        if op == 'X' then
            return 1
        else
            local a,b = e[1],e[2]
            if op == '+' then -- differentiation is linear
                local args = imap(diff,e,x)
                return balance(addi(args))
            elseif op == '*' then -- product rule
                local res,d,ee = {}
                for i = 1,#e do
                    d = fold(diff(e[i],x))
                    if d ~= 0 then
                        ee = {unpack(e)}
                        ee[i] = d
                        append(res,balance(muli(ee)))
                    end
                end
                if #res > 1 then return addi(res)
                else return res[1] end
            elseif op == '^' and isnumber(b) then -- power rule
                return b*x^(b-1)
            end
        end
    else
        return 0
    end
end



