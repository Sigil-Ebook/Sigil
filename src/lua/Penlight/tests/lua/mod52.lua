local test = require 'pl.test'

-- if STRICT is true, then M is distinct from _ENV, and ONLY contains
-- the exported functions!

local _ENV,M = require 'pl.import_into' (rawget(_G,'STRICT'))

function answer ()
    -- of course, you don't have the usual global environment available
    -- so define it as a local up above, or use utils.import(_G).
    test.assertraise(function()
        print 'hello'
    end,"attempt to call global 'print'")

    -- but all the Penlight modules are available
    return pretty.write(utils.split '10 20  30', '')
end

return M

