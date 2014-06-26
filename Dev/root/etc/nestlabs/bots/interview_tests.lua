--[[

Copyright 2011 by Nest Labs, Inc. All rights reserved.

This program is confidential and proprietary to Nest Labs, Inc.,
and may not be reproduced, published or disclosed to others without
company authorization.

--]]

package.path = package.path .. ";/nestlabs/etc/bots/?.lua"
require("sapphire_helper")

testSuite = {
    {
        internet        = 0,                    -- Internet Access?
        hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
        forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
        name            = "Oski Bear",          -- Thermostat name
        location        = "USA",                -- Location (USA or Canada)
        zip             = "94085",              -- ZIP code
        home            = 1,                    -- Home (1) or business (0)?
        homeAtNoon      = 0,                    -- Home at noon?
        bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
        count           = 3,                    -- # of Nest units in home
        learn           = 1,                    -- Auto adjust via learning
        heat_cool       = "heating"             -- heating or cooling
    },
    {
        internet        = 0,                    -- Internet Access?
        hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
        forcedAir       = 2,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
        name            = "Bedroom",            -- Thermostat name
        location        = "USA",                -- Location (USA or Canada)
        zip             = "95014",              -- ZIP code
        home            = 0,                    -- Home (1) or business (0)?
        openLate        = 1,                    -- Business open in the evenings?
        openSat         = 1,                    -- Business open Saturdays?
        openSun         = 0,                    -- Business open Sundays?
        count           = 1,                    -- # of Nest units in home
        learn           = 1,                    -- Auto adjust via learning
        heat_cool       = "cooling"             -- heating or cooling
    },
}


--[[
Test Case #1
       HOME: Home at Noon + In Bed by 11pm
             learning OFF
             Heating 
--]]
testSuite_001 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00001",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   


--[[
Test Case #2
       HOME: Home at Noon + In Bed by 11pm
             learning OFF
             Cooling 
--]]
testSuite_002 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00002",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   



--[[
Test Case #3
       HOME: Home at Noon + Not In Bed by 11pm
             learning OFF
             heating 
--]]
testSuite_003 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00003",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   



--[[
Test Case #4
       HOME: Home at Noon + Not In Bed by 11pm
             learning OFF
             Cooling 
--]]
testSuite_004 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00004",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   





--[[
Test Case #5
       HOME: Not Home at Noon + In bed by 11pm
             learning OFF
             heating 
--]]
testSuite_005 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00005",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 0,                    -- Home at noon?
       bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   




--[[
Test Case #6
       HOME: Not Home at Noon + In bed by 11pm
             learning OFF
             cooling 
--]]
testSuite_006 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00006",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 0,                    -- Home at noon?
       bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   



--[[
Test Case #7
       HOME: Not Home at Noon + Not In bed by 11pm
             learning OFF
             heating 
--]]
testSuite_007 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00007",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 0,                    -- Home at noon?
       bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #8
       HOME: Not Home at Noon + Not In bed by 11pm
             learning OFF
             cooling 
--]]
testSuite_008 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00008",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 0,                    -- Home at noon?
       bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   

--[[
Test Case #9
       BUSINESS: Open Evenings, learning OFF, heating
                 not open saturday
                 not open sunday
--]]
testSuite_009 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00009",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #10
       BUSINESS: Open Evenings, learning OFF, heating
                 open saturday
                 open saunday
--]]
testSuite_010 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00010",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #11
       BUSINESS: Open Evenings, learning OFF, heating
                 open saturday
                 not open sunday
--]]
testSuite_011 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00011",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #12
       BUSINESS: Open Evenings, learning OFF, heating
                 not open saturday
                 open sunday
--]]
testSuite_012 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00012",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #13
       BUSINESS: Open Evenings, learning OFF, cooling
                 not open saturday
                 not open sunday
--]]
testSuite_013 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00013",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   

--[[
Test Case #14
       BUSINESS: Open Evenings, learning OFF, cooling
                 open saturday
                 open sunday
--]]
testSuite_014 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00014",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   

--[[
Test Case #15
       BUSINESS: Open Evenings, learning OFF, cooling
                 open saturday
                 not open sunday
--]]
testSuite_015 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00015",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   
--[[
Test Case #16
       BUSINESS: Open Evenings, learning OFF, cooling
                 not open saturday
                 open sunday
--]]
testSuite_016 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00016",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 1,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   

--[[
Test Case #17
       BUSINESS: Not Open Evenings, learning OFF, heating
                 not open saturday
                 not open sunday
--]]
testSuite_017 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00017",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #18
       BUSINESS: Not Open Evenings, learning OFF, heating
                 open saturday
                 open sunday
--]]
testSuite_018 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00018",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #19
       BUSINESS: Not Open Evenings, learning OFF, heating
                 open saturday
                 not open sunday
--]]
testSuite_019 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00019",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #20
       BUSINESS: Not Open Evenings, learning OFF, heating
                 not open saturday
                 open sunday
--]]
testSuite_020 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00020",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #21
       BUSINESS: Not Open Evenings, learning OFF, cooling
                 not open saturday
                 not open sunday
--]]
testSuite_021 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00021",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   


--[[
Test Case #22
       BUSINESS: Not Open Evenings, learning OFF, cooling
                 open saturday
                 open sunday
--]]
testSuite_022 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00022",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   


--[[
Test Case #23
       BUSINESS: Not Open Evenings, learning OFF, cooling
                 open saturday
                 not open sunday
--]]
testSuite_023 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00023",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   


--[[
Test Case #24
       BUSINESS: Not Open Evenings, learning OFF, cooling
                 not open saturday
                 open sunday
--]]
testSuite_024 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00024",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   




--[[
Test Case #25
       HOME: Initial learning schedule.  learning ON, heating
--]]
testSuite_025 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00025",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 0,                    -- Home at noon?
       bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 1,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   


--[[
Test Case #26
       HOME: Initial learning schedule.  learning ON, cooling
--]]
testSuite_026 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00026",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 1,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   


--[[
Test Case #27
       BUSINESS: Initial learning schedule.  learning ON, heating
--]]
testSuite_027 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00027",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 1,                    -- Business open Saturdays?
       openSun         = 0,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 1,                    -- Auto adjust via learning
       heat_cool       = "heating"             -- heating or cooling
    },
}   

--[[
Test Case #28
       BUSINESS: Initial learning schedule.  learning ON, cooling
--]]
testSuite_028 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "electric",           -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00028",              -- ZIP code
       home            = 0,                    -- Home (1) or business (0)?
       openEvenings    = 0,                    -- Business open evenings?
       openSat         = 0,                    -- Business open Saturdays?
       openSun         = 1,                    -- Business open Sundays?
       count           = 1,                    -- # of Nest units in home
       learn           = 1,                    -- Auto adjust via learning
       heat_cool       = "cooling"             -- heating or cooling
    },
}   


--[[ 
Test Case #29 Initial learning
       Gas, ForcedAir, Home, HomeAtNoon, BedBy11pm, LEARN ON, HEATING
           
           1st Day Mon 05/09/2011: 02:00pm 62 (after interview)
                                   06:00pm 69
           2nd Day Tue 05/10/2011: 10:30am 68
                                   03:00pm 68
                                   03:30pm 70
           expected schedule result on 5/11/2011:
                    10:30am 68
                    12:30pm 62
                    03:00pm 70
                    05:00pm 62
                    06:00pm 69
                    08:00pm 62
--]]
       
testSuite_029 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00029",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 1,                    -- Auto adjust via learning
       heat_cool       = "heating",            -- heating or cooling
       dateTime        = "05091400201100"      -- MMDDhhmmYYYYss (MM-month, DD-day, YYYY-year, hh-hour (0-24), mm-minutes, ss-seconds
    },
}   

--[[ 
Test Case #30 Steady learning
       Home, HomeAtNoon, BedBy11pm, learning ON, cooling
       Day 1 Wed 6/15/2011: 08:00am 84
                            10:30am 84 to 71
       Day 2 Th  6/16/2011  
       Day 3 Fri 6/17/2011  
       Day 4 Sat 6/18/2011
       Day 5 Sun 6/19/2011 
       Day 6 Mon 6/20/2011
       Day 7 Tue 6/21/2011  02:18pm 84 to 75
       Day 8 Wed 6/22/2011  04:45pm 84 to 68
       Day 9 Wed 6/23/2011  04:45pm 84 to 68
       Day 10 expected schedule result:
                08:00am 84
                10:30am 71
                12:30pm 84
                02:18pm 75
                04:18pm 84
                04:45pm 68
                10:45pm 84
--]]
       
testSuite_030 = {
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
       learn           = 1,                    -- Auto adjust via learning
       heat_cool       = "cooling",            -- heating or cooling
       dateTime        = "06150800201100"      -- MMDDhhmmYYYYss (MM-month, DD-day, YYYY-year, hh-hour (0-24), mm-minutes, ss-seconds
    },
}

--[[
Test Case #31 Manual Temp set points
--]]
testSuite_031 = {
    {
       internet        = 0,                    -- Internet Access?
       hvacType        = "gas",                -- HVAC system type (electric, gas, probably electric, probably gas)
       forcedAir       = 1,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
       name            = "Nest",               -- Thermostat name
       location        = "USA",                -- Location (USA or Canada)
       zip             = "00031",              -- ZIP code
       home            = 1,                    -- Home (1) or business (0)?
       homeAtNoon      = 1,                    -- Home at noon?
       bedAt11pm       = 1,                    -- Everyone usually in bed by 11PM?
       count           = 1,                    -- # of Nest units in home
       learn           = 0,                    -- Auto adjust via learning
       heat_cool       = "cooling",            -- heating or cooling
       dateTime        = "05240800201100"      -- MMDDhhmmYYYYss (MM-month, DD-day, YYYY-year, hh-hour (0-24), mm-minutes, ss-seconds
    },
}

--[[
Test Case #batteryDrain
--]]
testSuite_batteryDrain = {
    {
        internet        = 0,                    -- Internet Access?
        ssid            = "Tiffany",            -- Internet SSID
        hvacType        = "dontknow",           -- HVAC system type (electric, gas, dontknow)
        forcedAir       = 2,                    -- Forced air? (1 - forced air, 2 - not forced air, 3 - don't know)
        name            = "Nest",               -- Thermostat name
        location        = "USA",                -- Location (USA or Canada)
        zip             = "09000",              -- ZIP code
        home            = 1,                    -- Home (1) or business (0)?
        homeAtNoon      = 0,                    -- Home at noon?
        bedAt11pm       = 0,                    -- Everyone usually in bed by 11PM?
        count           = 2,                    -- # of Nest units in home
        learn           = 0,                    -- Auto adjust via learning
        heat_cool       = "cooling"             -- heating or cooling
    },

}   




function runInterview(test, reset)
   for i,v in ipairs(test) do
     interviewTest(v, reset)
   end
end 


testCase = {
  [0] = function (x)
          -- Test Case #0
          -- run contineous testSuite
          runInterview(testSuite, 1)
        end,
  [1] = function (x)
          -- Test Case #1
          -- HOME: Home at Noon + In Bed by 11pm, learing OFF, heating
          runInterview(testSuite_001, 0)
          setLearning("off")
          -- view schedule
          -- verify: 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --   06am set to 68
          --   10pm set to 65
          goToMenu("schedule")
        end,
  [2] = function (x)
          -- Test Case #2
          -- HOME: Home at Noon + In Bed by 11pm, learing OFF, cooling
          runInterview(testSuite_002, 0)
          setLearning("off")
          -- view schedule
          -- verify: 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --   08am set to 79
          --   06pm set to 76
          goToMenu("schedule")
        end,
  [3] = function (x)
          -- Test Case #3
          -- HOME: Home at Noon + Not In Bed by 11pm, learing OFF, heating
          runInterview(testSuite_003, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --   12am set to 65
          --   06am set to 68
          goToMenu("schedule")
        end,
  [4] = function (x)
          -- Test Case #4
          -- HOME: Home at Noon + Not In Bed by 11pm, learing OFF, cooling
          runInterview(testSuite_004, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --    08am set to 79
          --    06pm set to 76
          goToMenu("schedule")
        end,
  [5] = function (x)
          -- Test Case #5
          -- HOME: Not Home at Noon + In Bed by 11pm, learing OFF, heating
          runInterview(testSuite_005, 0)
          setLearning("off")
          -- view schedule
          -- Verify 4 Set Points for all 7 days (M T W Th F Sa Su) 
          --    06am set to 68
          --    08am set to 62
          --    06pm set to 68
          --    10pm set to 65
          goToMenu("schedule")
        end,
  [6] = function (x)
          -- Test Case #
          -- HOME: Not Home at Noon + In Bed by 11pm, learing OFF, cooling
          runInterview(testSuite_006, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --    08am set to 84
          --    06pm set to 76
          goToMenu("schedule")
        end,
  [7] = function (x)
          -- Test Case #7
          -- HOME: Not Home at Noon + Not In Bed by 11pm, learing OFF, heating
          runInterview(testSuite_007, 0)
          setLearning("off")
          -- view schedule
          -- Verify 4 Set Points for all 7 days (M T W Th F Sa Su) 
          --    12am set to 65
          --    06am set to 68
          --    08am set to 62
          --    06pm set to 68
          goToMenu("schedule")
        end,
  [8] = function (x)
          -- Test Case #8 
          -- HOME: Not Home at Noon + Not In Bed by 11pm, learing OFF, cooling
          runInterview(testSuite_008, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 set points for Mon through Fri & any opening Sat, Sun
          --    8am set to 84
          --    6pm set to 76
          goToMenu("schedule")
        end,
  [9] = function (x)
          -- Test Case #9 
          -- BUSINESS: Open Evenings, learning OFF, heating
          --           not open Saturday
          --           not open Sunday
          runInterview(testSuite_009, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      09pm set to 60  (M - F)
          --      Saturday & Sunday 12am to 60
          goToMenu("schedule")
        end,
  [10] = function (x)
          -- Test Case #10 
          -- BUSINESS: Open Evenings, learning OFF, heating
          --           open Saturday
          --           open Sunday
          runInterview(testSuite_010, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      09pm set to 60  (M - F)
          --      same for Saturday & Sunday
          goToMenu("schedule")
        end,
  [11] = function (x)
          -- Test Case #11 
          -- BUSINESS: Open Evenings, learning OFF, heating
          --           open Saturday
          --           not open Sunday
          runInterview(testSuite_011, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      09pm set to 60  (M - F)
          --      same for Saturday
          --      Sunday 12am to 60
          goToMenu("schedule")
        end,
  [12] = function (x)
          -- Test Case #12 
          -- BUSINESS: Open Evenings, learning OFF, heating
          --           not open Saturday
          --           open Sunday
          runInterview(testSuite_012, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      09pm set to 60  (M - F)
          --      Saturday 12am to 60 
          --      Sunday same as M-F
          goToMenu("schedule")
        end,
  [13] = function (x)
          -- Test Case #13
          -- BUSINESS: Open Evenings, learning OFF, cooling
          --           not open Saturday
          --           not open Sunday
          runInterview(testSuite_013, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76 (M-F)
          --      09pm set to 84 (M-F)
          --      Saturday and Sunday 12am to 84
          goToMenu("schedule")
        end,
  [14] = function (x)
          -- Test Case #14
          -- BUSINESS: Open Evenings, learning OFF, cooling
          --           open Saturday
          --           open Sunday
          runInterview(testSuite_014, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76 (M-F)
          --      09pm set to 84 (M-F)
          --      same for Saturday and Sunday
          goToMenu("schedule")
        end,
  [15] = function (x)
          -- Test Case #15
          -- BUSINESS: Open Evenings, learning OFF, cooling
          --           open Saturday
          --           not open Sunday
          runInterview(testSuite_015, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76 (M-F)
          --      09pm set to 84 (M-F)
          --      same for Saturday
          --      Sunday 12am  to 84
          goToMenu("schedule")
        end,
  [16] = function (x)
          -- Test Case #16
          -- BUSINESS: Open Evenings, learning OFF, cooling
          --           not open Saturday
          --           open Sunday
          runInterview(testSuite_016, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76 (M-F)
          --      09pm set to 84 (M-F)
          --      Saturday 12am to 84
          --      Sunday same as M-F
          goToMenu("schedule")
        end,
  [17] = function (x)
          -- Test Case #17
          -- BUSINESS: Not Open Evenings, learning OFF, heating
          --           not open saturday
          --           not open sunday
          runInterview(testSuite_017, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F)
          --      06pm set to 60  (M - F)
          --      Saturday & Sunday 12am to 60
          goToMenu("schedule")
        end,
  [18] = function (x)
          -- Test Case #18
          -- BUSINESS: Not Open Evenings, learning OFF, heating
          --           open saturday
          --           open sunday
          runInterview(testSuite_018, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      06pm set to 60  (M - F)
          --      same for saturday and sunday
          goToMenu("schedule")
        end,
  [19] = function (x)
          -- Test Case #19
          -- BUSINESS: Not Open Evenings, learning OFF, heating
          --           open saturday
          --           not open sunday
          runInterview(testSuite_019, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      06pm set to 60  (M - F)
          --      same for Saturday
          --      Sunday 12am to 60
          goToMenu("schedule")
        end,
  [20] = function (x)
          -- Test Case #20
          -- BUSINESS: Not Open Evenings, learning OFF, heating
          --           not open saturday
          --           open sunday
          runInterview(testSuite_020, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 68  (M - F) 
          --      06pm set to 60  (M - F)
          --      Saturday 12am to 60
          --      Sunday same as M-F
          goToMenu("schedule")
        end,
  [21] = function (x)
          -- Test Case #21 
          -- BUSINESS: Not Open Evenings, learning OFF, cooling
          --           not open saturday
          --           not open sunday
          runInterview(testSuite_021, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76  (M - F) 
          --      06pm set to 84  (M - F)
          --      Saturday & Saturday 12am to 84
          goToMenu("schedule")
        end,
  [22] = function (x)
          -- Test Case #22 
          -- BUSINESS: Not Open Evenings, learning OFF, cooling
          --           open saturday
          --           open sunday
          runInterview(testSuite_022, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76  (M - F) 
          --      06pm set to 84  (M - F)
          --      same for Saturday & Sunday
          goToMenu("schedule")
        end,
  [23] = function (x)
          -- Test Case #23 
          -- BUSINESS: Not Open Evenings, learning OFF, cooling
          --           open saturday
          --           not open sunday
          runInterview(testSuite_023, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76  (M - F) 
          --      06pm set to 84  (M - F)
          --      same for Saturday
          --      Sunday 12am to 84
          goToMenu("schedule")
        end,
  [24] = function (x)
          -- Test Case #24 
          -- BUSINESS: Not Open Evenings, learning OFF, cooling
          --           not open saturday
          --           open sunday
          runInterview(testSuite_024, 0)
          setLearning("off")
          -- view schedule
          -- Verify 2 Set Points for all 7 days (M T W Th F Sa Su) 
          --      09am set to 76  (M - F) 
          --      06pm set to 84  (M - F)
          --      Saturday 12am to 84
          --      Sunday same as M-F
          goToMenu("schedule")
        end,
  [25] = function (x)
          -- Test Case #25
          -- HOME: Initial Learning Schedule - learning ON, heating
          runInterview(testSuite_025, 0)
          -- view schedule
          -- Verify 1 Set Points for all 7 days (M T W Th F Sa Su) 
          --      8am set to 62 
          goToMenu("schedule")
        end,
  [26] = function (x)
          -- Test Case #26 
          -- HOME: Initial Learning Schedule - learning ON, cooling
          runInterview(testSuite_026, 0)
          -- view schedule
          -- Verify 1 Set Points for all 7 days (M T W Th F Sa Su)
          --      8am set to 84
          goToMenu("schedule")
        end,
  [27] = function (x)
          -- Test Case #27 
          -- BUSINESS: Initial Learning Schedule - learning ON, heating
          runInterview(testSuite_027, 0)
          -- view schedule
          -- Verify 1 Set Points for all 7 days (M T W Th F Sa Su) 
          --      8am set to 62 
          goToMenu("schedule")
        end,
  [28] = function (x)
          -- Test Case #28 
          -- BUSINESS: Initial Learning Schedule - learning ON, cooling
          runInterview(testSuite_028, 0)
          -- view schedule
          -- Verify 1 Set Points for all 7 days (M T W Th F Sa Su) 
          --      8am at 84
          goToMenu("schedule")
        end,
  [29] = function (x) 
          --[[ 
             Test Case #29 Initial Learning
             Home: HomeAtNoon, BedBy11pm, LEARN ON, HEATING
             
             1st Day Mon 05/09/2011: 02:00pm 62 (after interview)
                                     06:00pm 69
             2nd Day Tue 05/10/2011: 10:30am 68
                                     03:00pm 68
                                     03:30pm 70
             expected schedule result on 5/11/2011:
                      10:30am 68
                      12:30pm 62
                      03:00pm 70
                      05:00pm 62
                      06:00pm 69
                      08:00pm 62
           --]]
        
           runInterview(testSuite_029, 0)
           print("TEST Case 29: interview completed!")
           
           -- change time: 5/9/2011 06:00pm
           setSystemDateTime("05091800201100")
           setTemp(62, 69)
           
           -- Temperature set to 69 till hh:mm
           sleep(5000)
           click()

           -- verify temp at heat 69
           checkCurrentTargetTemp("heat", 69)

           -- advance 1 day to 5/10/2011
           --   advanceDays(1)
           advanceSystemDays(1)
           
           -- change time: 5/10/2011 10:30am, change temp 62 to 68
           setSystemDateTime("05101030201100")
           -- setTemp("69","62", nil)
           setTemp(62, 68)

           -- verify temp at heat 68
           checkCurrentTargetTemp("heat", 68)

           -- change time: 5/10/2011 03:00pm, change temp 62 to 68
           setSystemDateTime("05101500201100")
           -- setTemp("68","62", nil)
           setTemp(62, 68)

           -- verify temp at heat 68
           checkCurrentTargetTemp("heat", 68)

           -- change time: 5/10/2011 03:30pm, change temp 62 to 70
           setSystemDateTime("05101530201100")
           -- setTemp("68","62", nil)
           setTemp(62, 70)

           -- verify temp at heat 70
           checkCurrentTargetTemp("heat", 70)
           
           -- advance 1 day to 5/11/2011
           advanceSystemDays(1)

           -- view schedule
           goToMenu("schedule",0)

        end,
  [30] = function (x)  
           --[[
             Home, HomeAtNoon, BedBy11pm, learning ON, cooling
             Day 1 Wed 6/15/2011: 08:00am 84
                                  10:30am 84 to 71
             Day 2 Th  6/16/2011  
             Day 3 Fri 6/17/2011  
             Day 4 Sat 6/18/2011
             Day 5 Sun 6/19/2011 
             Day 6 Mon 6/20/2011
             Day 7 Tue 6/21/2011  02:18pm 84 to 75
             Day 8 Wed 6/22/2011  04:45pm 84 to 68
             Day 9 Wed 6/23/2011  04:45pm 84 to 68
             Day 10 Th 6/24/2022  expected schedule result:
                                    08:00am 84
                                    10:30am 71
                                    12:30pm 84
                                    02:18pm 75
                                    04:18pm 84
                                    04:45pm 68
                                    10:45pm 84
           --]]

           runInterview(testSuite_030, 0)
           print("TEST Case 30: interview completed!")

           -- change time: 6/15/2011 10:30am
           setSystemDateTime("06151030201100")
           setTemp(84, 71)

           -- Temperature set to 69 till hh:mm
           sleep(5000)
           click()

           -- advance 6 days to 6/21/2011
           -- advanceDays(6)
           advanceSystemDays(6)

           -- change time: 6/21/2011 02:18pm, change temp 84 to 75
           setSystemDateTime("06211418201100")
           -- setTemp("71","84", nil)
           setTemp(84, 75)
           
           -- advance 1 day to 6/22/2011
           advanceSystemDays(1)

           -- change time: 6/22/2011 04:45pm, change temp 84 to 68
           setSystemDateTime("06221645201100")
           -- setTemp("75","84", nil)
           setTemp(84, 68)
           
           -- advance 1 day to 6/23/2011
           advanceSystemDays(1)

           -- change time: 6/23/2011 04:45pm, change temp 84 to 68
           setSystemDateTime("06231645201100")
           -- setTemp("68","84", nil)
           setTemp(84, 68)
           
           -- advance 1 day to 6/24/2011
           advanceSystemDays(1)

           -- view schedule
           goToMenu("schedule",0)
         end,
  [31] = function (x)  
           --[[
             Maual setpoint holds for 4hrs
             Home, home at noon, in bed by 11pm, cooling
             Day 1 Tue 5/24/2011  8am 79 (starting time and starting temp)
                                 10am change temp from 79 to 65
             expected results: 
                               11:00am temp is 65
                               12:00pm temp is 65
                                1:00pm temp is 65
                                1:59pm temp is 65
                                2:00pm temp is changed to 79
           --]]  
           runInterview(testSuite_031, 0)
           print("TEST Case 31: interview completed!")
           
           -- change time: 5/24/2011 11:00am
           setSystemDateTime("05241100201100")
           setTemp(79, 65)

           -- Temperature set to 69 till hh:mm
           sleep(5000)
           click()
           
           -- change time: 5/24/2011 12:00pm
           setSystemDateTime("05241200201100")
           -- verify temp at 65?
           checkCurrentTargetTemp("cool",65)
           
           -- change time: 5/24/2011 13:00pm
           setSystemDateTime("05241300201100")
           -- verify temp at 65?
           checkCurrentTargetTemp("cool", 65)

           -- change time: 5/24/2011 13:59pm
           setSystemDateTime("05241359201100")
           -- verify temp at 65?
           checkCurrentTargetTemp("cool", 65)
           
           -- change time: 5/24/2011 13:59pm
           setSystemDateTime("05241359201100")

           -- verify temp at 79
           checkTargetTempChanged("cool", 79)
         end,
  [32] = function (x)  end,
  [9000] = function (x)
           runInterview(testSuite_batteryDrain, 0)
           print("TEST Case 31: interview completed!")

           goToMenu("settings", "techInfo")
           count = 1
           while count > 0 do
             for i=1, 20, 1 do
               rotateDial(2)
               sleep(1000)
             end
             for j=1, 20, 1 do
               rotateDial(-2)
               sleep(1000)
             end
             count = count - 1
           end
           backToMain("settings", "techInfo")
         end,
}


--[[ 
      SELECT A TEST CASE TO RUN 
--]]

-- testCase[0](testSuite)
-- testCase[1](testSuite_001)
-- testCase[2](testSuite_002)
-- testCase[3](testSuite_003)
-- testCase[4](testSuite_004)
-- testCase[5](testSuite_005)
-- testCase[6](testSuite_006)
-- testCase[7](testSuite_007)
-- testCase[8](testSuite_008)
-- testCase[9](testSuite_009)
-- testCase[10](testSuite_010)
-- testCase[11](testSuite_011)
-- testCase[12](testSuite_012)
-- testCase[13](testSuite_013)
-- testCase[14](testSuite_014)
-- testCase[15](testSuite_015)
-- testCase[16](testSuite_016)
-- testCase[17](testSuite_017)
-- testCase[18](testSuite_018)
-- testCase[19](testSuite_019)
-- testCase[20](testSuite_020)
-- testCase[21](testSuite_021)
-- testCase[22](testSuite_022)
-- testCase[23](testSuite_023)
-- testCase[24](testSuite_024)
-- testCase[25](testSuite_025)
-- testCase[26](testSuite_026)
-- testCase[27](testSuite_027)
-- testCase[28](testSuite_028)
-- testCase[29](testSuite_029)
-- testCase[30](testSuite_030)
-- testCase[31](testSuite_031)
-- testCase[9000](testSuite_batteryDrain)
