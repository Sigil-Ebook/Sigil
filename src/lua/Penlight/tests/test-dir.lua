-- This test file expects to be ran from 'run.lua' in the root Penlight directory.

local dir = require( "pl.dir" )
local file = require( "pl.file" )
local path = require( "pl.path" )
local asserteq = require( "pl.test" ).asserteq
local pretty = require( "pl.pretty" )

local normpath = path.normpath

local expected = {normpath "../doc/config.ld"}

local files = dir.getallfiles( normpath "../doc/", "*.ld" )

asserteq( files, expected )

-- Test move files -----------------------------------------

-- Create a dummy file
local fileName = path.tmpname()
file.write( fileName, string.rep( "poot ", 1000 ) )

local newFileName = path.tmpname()
local err, msg = dir.movefile( fileName, newFileName )

-- Make sure the move is successful
assert( err, msg )

-- Check to make sure the original file is gone
asserteq( path.exists( fileName ), false )

-- Check to make sure the new file is there
asserteq( path.exists( newFileName ) , newFileName )

-- Try to move the original file again (which should fail)
local newFileName2 = path.tmpname()
local err, msg = dir.movefile( fileName, newFileName2 )
asserteq( err, false )

-- Clean up
file.delete( newFileName )


-- Test copy files -----------------------------------------

-- Create a dummy file
local fileName = path.tmpname()
file.write( fileName, string.rep( "poot ", 1000 ) )

local newFileName = path.tmpname()
local err, msg = dir.copyfile( fileName, newFileName )

-- Make sure the move is successful
assert( err, msg )

-- Check to make sure the new file is there
asserteq( path.exists( newFileName ) , newFileName )

-- Try to move a non-existant file (which should fail)
local fileName2 = path.tmpname()
local newFileName2 = path.tmpname()
local err, msg = dir.copyfile( fileName2, newFileName2 )
asserteq( err, false )

-- Clean up the files
file.delete( fileName )
file.delete( newFileName )


-- have NO idea why forcing the return code is necessary here (Windows 7 64-bit)
os.exit(0)

