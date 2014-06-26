--[[

Copyright 2011 by Nest Labs, Inc. All rights reserved.

This program is confidential and proprietary to Nest Labs, Inc.,
and may not be reproduced, published or disclosed to others without
company authorization.

--]]

package.path = package.path .. ";/nestlabs/etc/bots/?.lua"
require("xmlparser")

-- This specifies the location of the settings output (should be /nestlabs/etc/user/settings.config on a real device)
settingsConfig = "/nestlabs/etc/user/settings.config"

-----------------------------------------------------------------------------------------
-- UI HELPERS
-----------------------------------------------------------------------------------------
splashGuard             = 4000
clickInterval           = 900
scrollInterval          = 50
tempScrollInterval      = 15

function click()
    clickDown()
    clickUp()
    sleep(clickInterval)
end

function scrollItem(n)
    while n ~= 0 do
        if n > 0 then
            rotateDial(4)
            n = n - 1
        else
            rotateDial(-4)
            n = n + 1
        end
        sleep(scrollInterval)
    end
end

function scrollDegrees(n)
    n = n * 2
    while n ~= 0 do
        if n > 0 then
            rotateDial(27/40)
            n = n - 1
        else
            rotateDial(-27/40)
            n = n + 1
        end
        sleep(tempScrollInterval)
    end
end

function setSetpoint(t)
    scrollDegrees(t-setpoint)
    setpoint = t
end

function setSetpointInstant(t)
    diff = t-setpoint
    rotateDial(diff * (54/40))
    setpoint = t
end

backLightBrightness = "/sys/devices/platform/pwm-backlight.0/backlight/pwm-backlight.0/brightness"

function setBrightness(b)
    f = io.open(backLightBrightness, "w")
    f:write(string.format("%d", b))
    f:close()
end

function sigmoid(t)
    t = (t * 12 / 100) - 6
    return 100 / (1 + math.exp(-t))
end

-- We need to eventually replace the linear dimming with an S-curve transition
function hideScreen()
    sleep(3000)
    for i=1,100 do
       setBrightness(100-sigmoid(i)) 
       sleep(10)
    end
end

function showScreen()
    sleep(3500)
    for i=1,100 do
       setBrightness(sigmoid(i)) 
       sleep(10)
    end
    sleep(3000)
end

function f2c(f)                                                                          
    return (f - 32) * 5 / 9                                                              
end   

function switchHeatCool()
    click()
    scrollItem(-1)
    click()
end

function recalibrateDial()
    -- Keep dialing down to 50 degrees while we're waiting for client to fully load
    rotateDial(-1000)
    setpoint = 50
end

-----------------------------------------------------------------------------------------
-- DEMO
-----------------------------------------------------------------------------------------

interviewCompleted = 0

fileSettingsConfig = io.open(settingsConfig, "r")
if fileSettingsConfig ~= nil then
    xmlTree = XmlParser:ParseXmlText(fileSettingsConfig:read("*all"))
    settings={}
    for i,xmlNode in pairs(xmlTree.ChildNodes) do
        if (xmlNode.Name == "s") then
            logDebug(string.format("key=%s value=%s", xmlNode.Attributes.key, xmlNode.Attributes.value))
            settings[xmlNode.Attributes.key] = xmlNode.Attributes.value
        end
    end
    fileSettingsConfig:close()
    interviewCompleted = tonumber(settings["interview_completed"])
end

-- Skip the demo loop if user has not completed interview
if interviewCompleted == 1 then

    -- Set demo mode frequency to 600MHz to smooth out animations
    f = io.open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "w")
    f:write("600000")
    f:close()
    
    setBrightness(0)

    -- Nest splash screen
    sleep(splashGuard)
    
    --[[
    -- Get past wire change warning
    click()
    ]]--

    -- Click through date setting because we're not connected to WiFi
    click()
    click()
    click()
    click()
    click()
    click()
    click()
    click()

    sleep(1000)

    -- Make sure we start in heating mode
    if (settings["last_nonrange_sp"] ~= "HEAT") then
        switchHeatCool()
    end
    
    while (1) do
        recalibrateDial()
    
        -- Trigger AUX pin here to synchronize Nest storyboard with Rapid storyboard
        -- Show logo screen while we're here (we might want to get rid of the spinning dial)
        jumpToScreen("logo")
        setAuxPin(1)        
        showScreen()
        sleep(2000)
        hideScreen()
        setAuxPin(0)
        jumpToScreen("temperature")
        sleep(1000)
    
        -- 1. COMFORT
        setSetpointInstant(63)
        setSimTemp(f2c(68))
        showScreen()
        -- 1.a Icon Lighting: Heating icon comes on sharply:
        -- 1.b Thermostat: 63 target (black) to 70 target (red)].  Pause.
        setSetpoint(70)
        -- 1.c House Lighting: house starts softly glowing red when temperature hits 65 target, more boldly as it continues to 70 target.
        sleep(2000)
        -- 1.d Thermostat: Screen goes black
        hideScreen()
    
        -- 2. CONSERVATION
        setSetpointInstant(63)
        showScreen()
        -- 2.a Icon Lighting: Leaf icon comes on sharply:
        -- 2.b Thermostat: [Thermostat turns from 63 (black with leaf) to 80 (red/leaf disappears), pause...and back down to 70 (red) to get the green leaf icon back.] 
        setSetpoint(80)
        sleep(3000)
        setSetpoint(70)
        sleep(2000)
        -- 2.c House Lighting: house glows softly red at 72, bright to 75, back to soft at 72.  
        -- 2.d Thermostat: Screen goes black 
        hideScreen()
    
        -- 3. CONSERVATION
        click()
        scrollItem(1)
        click()
        showScreen()
        -- 3.a Icon Lighting: Away mode icon comes on sharply
        -- 3.b House Lighting: Yellow lights in house go off sharply
        -- 3.c Thermostat:, 
        sleep(2000)
        -- 3.d Thermostat: screen goes black 
        hideScreen()
        click()
    
        -- 4. CONTROL
        -- Change to cooling mode
        switchHeatCool()
        setSetpointInstant(85)
        setSimTemp(f2c(80))
        showScreen()
        -- 4.a Icon Lighting: Remote control icon lights up sharply
        -- 4.b Thermostat: [Thermostat turns from 85 (black with leaf) to 70 (blue) leaf disappears.]    
        setSetpoint(70)
        -- 4.c House lighting: House glows softly blue at 80, more brightly at 74. 
        -- 4.d Icon lighting: as thermostat going blue, cooling icon by comfort comes on.
        -- 4.e Screen goes black 
        -- 4.f Return to Nest Logo
        hideScreen()
        -- Change to heating mode
        switchHeatCool()
    end
end
