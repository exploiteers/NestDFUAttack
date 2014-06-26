--[[

Copyright 2011 by Nest Labs, Inc. All rights reserved.

This program is confidential and proprietary to Nest Labs, Inc.,
and may not be reproduced, published or disclosed to others without
company authorization.

--]]

package.path = package.path .. ";/nestlabs/etc/bots/?.lua"
require("sapphire_helper")

conf =
{
    internet        = 0,                    -- Internet Access?
    hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
    forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
    name            = "Nest",               -- Thermostat name
    location        = "USA",                -- Location (USA or Canada)
    zip             = "00030",              -- ZIP code
    home            = 1,                    -- Home (1) or business (0)?
    homeAtNoon      = 1,                    -- Home at noon?
    bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
    count           = 1,                    -- # of Nest units in home
    learn           = 0,                    -- Auto adjust via learning
    heat_cool       = "cooling",            -- heating or cooling
    dateTime        = "05240800201100"      -- MMDDhhmmYYYYss (MM-month, DD-day, YYYY-year, hh-hour (0-24), mm-minutes, ss-seconds
}

interviewTest(conf, 0)

scrollInterval = scrollInterval * 8

-- We should be in the temperature screen at this point
click()

-- (DONE)   COOL   SCHEDULE   ENERGY   SETTINGS   AWAY
scrollItem(2)

--  DONE   COOL   SCHEDULE   ENERGY   (SETTINGS)   AWAY
click()

-- (DONE)   FAN   LOCK   LEARNING   AUTO-AWAY   BRIGHTNESS   CLICK SOUND   F/C   NAME   NETWORK   NEST ACCOUNT   ZIP CODE   DATE & TIME ...
scrollSettings(10)
sleep(1000)

--  DONE    FAN   LOCK   LEARNING   AUTO-AWAY   BRIGHTNESS   CLICK SOUND   F/C   NAME   NETWORK  (NEST ACCOUNT)  ZIP CODE   DATE & TIME ...
click()

-- Ready to get a secure passcode from nest.com (not sure why this requires two clicks)
-- (GET PASSCODE)
-- DONE
click()

-- Retrieving passcode
sleep(10000)

-- Your Nest passcode is:
-- 000-AAAA
-- valid for 4 hours
click()

-- Please create an account at nest.com and enter the passcode:
-- 000-AAAA
-- (DONE)
