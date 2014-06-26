--[[

Copyright 2011 by Nest Labs, Inc. All rights reserved.

This program is confidential and proprietary to Nest Labs, Inc.,
and may not be reproduced, published or disclosed to others without
company authorization.

--]]

package.path = package.path .. ";/nestlabs/etc/bots/?.lua"

require("sapphire_helper")
require("interview_tests")

local socket = require('socket')

-- create a TCP socket and bind it to the local host, at any port
local server = assert(socket.bind("*", 8000))

-- find out which port the OS chose for us
local ip, port = server:getsockname()

function wrapper(f, client)
    results = f()
    if (results == nil) then
        results = "0"
    end
    client:send(results .. "\n")
end

-- loop until Lua bot told to exit
looping = true
while looping do
    -- wait for a connection from any client
    local client = server:accept()
    -- make sure we don't block waiting for this client's line
    client:settimeout(60)
    -- if there was no error, send it back to the client
    while true do
        -- receive the line
        local line, err = client:receive()

        if line == "exit" then
            looping = false
            break
        elseif not err then
            logDebug("Executing lua line: '" .. line .. "'\n")
            pcall(wrapper, loadstring("return ".. line), client)
        else
            logDebug("Closing connection\n")
            break
        end
    end
    -- done with client, close the object
    client:close()
end
