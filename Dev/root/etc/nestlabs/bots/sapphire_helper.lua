--[[

Copyright 2011 by Nest Labs, Inc. All rights reserved.

This program is confidential and proprietary to Nest Labs, Inc.,
and may not be reproduced, published or disclosed to others without
company authorization.

--]]

package.path = package.path .. ";/nestlabs/etc/bots/?.lua"
require("xmlparser")

platform = io.popen("uname -s"):read("*l")

if platform == "Darwin" then
   -- on the mac simulator, use this path:
   tempScreenOutput = "/nestlabs/var/data/tempscreenoutput"
else
   -- on the device, use/un-comment this path:
   tempScreenOutput = "/media/data/tempscreenoutput"
end

settingsConfig  = "/nestlabs/etc/user/settings.config"

-----------------------------------------------------------------------------------------
-- UI HELPERS
-----------------------------------------------------------------------------------------
splashGuard     = 10000
clickInterval   = 1200
scrollInterval  = 60

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

function scrollSettings(n)
    while n ~= 0 do
        if n > 0 then
            rotateDial(4)
            n = n - 1
        else
            rotateDial(-4)
            n = n + 1
        end
        sleep(200)
    end
end

function scrollTemp(n)
    while n ~= 0 do
        if n > 0 then
            rotateDial(1.35)
            n = n - 1
        else
            rotateDial(-1.35)
            n = n + 1
        end
        sleep(scrollInterval)
    end
end


function setZip(zip, defaultZip, digits)
    for i=1,digits do
        scrollItem(string.byte(zip, i) - string.byte(defaultZip, i))
        click()
    end
end

function setZipUS(zip)
    if zip == nil then
        for i=1,5 do click() end
    else
        setZip(zip, "94301", 5)
    end
    click()
end

function setZipCanada(zip)
    setZip(zip, "A5A5A5", 6)
end

dialAlphaLower = {"del","case","back","done","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"}
dialAlphaUpper = {"del","case","back","done","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"}
dialNumber     = {"del","case","back","done","0","1","2","3","4","5","6","7","8","9","-","!","@","#","$","%","^","&","*","(",")","_","+","=","{","}"}
dialSymbol     = {"del","case","back","done","~","[","]","|"," ","\\",'"', ":",";","'","<",">","?",",",".","/","`"}
dials          = {dialAlphaLower, dialAlphaUpper, dialNumber, dialSymbol}

function dialHasChar(dial, c)
    for i,v in ipairs(dial) do
        if dial[i] == c then
            return true
        end
    end
    return false
end

dialIdxI = 1
dialIdxJ = 4

function searchDialUntilChar(c)
    scrollAmount = 0
    currentDial = dials[dialIdxI]
    while (dials[dialIdxI][dialIdxJ] ~= c) do
        if (dialIdxJ == #currentDial) then
            dialIdxJ = 1
        else
            dialIdxJ = dialIdxJ + 1
        end
        scrollAmount = scrollAmount + 1
    end
    if ((2*scrollAmount) > #currentDial) then
        scrollAmount = scrollAmount - #currentDial
    end
    scrollItem(-scrollAmount)
end

function gotoDialOfChar(c)
    while (dialHasChar(dials[dialIdxI], c) == false) do
        searchDialUntilChar("case")
        click()
        if (dialIdxI == 4) then
            dialIdxI = 1
        else
            dialIdxI = dialIdxI + 1
        end
    end
end

function setChar(c)
    gotoDialOfChar(c)
    searchDialUntilChar(c)
    click()
end

function setText(t)
    scrollInterval  = scrollInterval * 3
    clickInterval = clickInterval / 10
    for i=1,string.len(t) do
        setChar(string.sub(t, i, i))
    end
    setChar("done")
    scrollInterval  = scrollInterval / 3
    clickInterval = clickInterval * 10
    sleep(1000)
end

function setName(name)
    preCookedNames = {}
    preCookedNames["Nest"]              = 0
    preCookedNames["Hallway"]           = 1
    preCookedNames["Living room"]       = 2
    preCookedNames["Bedroom"]           = 3
    preCookedNames["Upstairs"]          = 4
    preCookedNames["Downstairs"]        = 5
    preCookedNames["Office"]            = 1
    preCookedNames["Main"]              = 2
    if (preCookedNames[name] == nil) then
        scrollItem(6)
        click()
        setText(name)
    else
        scrollItem(preCookedNames[name])
        click()
    end
end

function goToMenu(menu, submenu)
    -- menu
    --   (done)
    --   -1 coolHeat
    --   -2 schedule
    --   -3 energy
    --    2 settings
    --    1 away
    print("goToMenu:", menu, submenu)
    scrollInterval  = scrollInterval * 30
    click()

    if (menu == "energy") then
        scrollItem(-3)
        click()
    elseif (menu == "schedule") then
        scrollItem(-2)
        click()
        sleep(6000)
        -- scroll & view schedule from Monday to Sunday
        rotateDial(-20)
        rotateDial(3.155)  -- align to Monday 12pm
        sleep(4000)
        count = 6
        while (count > 0) do
          for i=1, 28, 1 do
            rotateDial(1)
            sleep(200)
          end
          rotateDial(0.7)
          sleep(3000)
          count = count - 1
        end
        rotateDial(-20)
        sleep(5000)
        click()
    elseif (menu == "settings") then
        scrollItem(2)
        click()
        -- settings submenu:
        --    1 fan
        --    2 lock
        --    3 learning/schedule
        --    4 autoAway
        --    5 brightness
        --    6 clickSound
        --    7 fC
        --    8 name
        --    9 network
        --   10 nestAcct
        --   11 zipCode
        --   12 dateTime
        --   13 installation
        --   14 advancedSetting
        --   15 techInfo
        --   16 legal
        --   17 reset
        if (submenu == "reset") then
            scrollSettings(17)
            click()
        elseif (submenu == "dateTime") then
            scrollSettings(12)
            click()
        elseif (submenu == "techInfo") then
            scrollSettings(15)
            sleep(2000)
            click()
        elseif (submenu == "learning") then
            scrollSettings(3)
            sleep(2000)
            click()
        end
    elseif (menu == "coolHeat") then
        scrollItem(-1)
        click()
    elseif (menu == "away") then
        scrollItem(1)
        click()
    end
    scrollInterval  = scrollInterval / 30
end

function backToMain(menu, submenu)
    -- given current menu/submenu, scroll back to main screen
    scrollInterval  = scrollInterval * 30
    if (menu == "settings") then
       if (submenu == "dateTime") then
          scrollSettings(-12)
       elseif (submenu == "techInfo") then
          click()
          scrollSettings(-15)
          sleep(2000)
       elseif (submenu == "learning") then
          scrollSettings(-3)
       end
    end
    click()
    scrollInterval  = scrollInterval / 30
end

function reset(type)
    -- (Cancel)
    -- Learning
    -- To Default = all 
    -- Restart
    goToMenu("settings", "reset")
    if (type == "all") then
        scrollItem(2)
        sleep(2000)
        click()
        scrollItem(1)
        sleep(2000)
        click()
    elseif (type == "learning") then
        scrollItem(1)
        sleep(2000)
        click()
        scrollItem(1)
        sleep(2000)
        click()
    elseif (type == "restart") then
        scrollItem(3)
        sleep(2000)
        click()
    end
    scrollItem(10)
    click()
    sleep(15000)
end

function switchMode(mode)
   goToMenu("coolHeat")
   sleep(2000)
   scrollItem(-5) --scroll to the top
   -- range (if range enabled)
   -- heat
   -- cool
   -- away (if range enabled)
   -- off
   if (mode == "cool") then
       scrollItem(2)
   elseif (mode == "off") then
       scrollItem(3)
   end
   click()
end

function setLearning(learn)
    -- (YES) (Continue)
    -- (No)  (Turn Off Learning)
    print("setLearning:", learn)
    goToMenu("settings", "learning")
    if (learn == 'off') then
        scrollItem(2)
        click()
        sleep(5000)
    elseif (learn == "on") then
        click()
        sleep(1000)
        click()
        sleep(5000)
    end
    backToMain("settings", "learning")
end

function setAwayLowTemp(s)
    -- cooling: 70 to 90
    -- heating: 55 to 75
    print("setAwayLowTemp =", s.awayLowTemp)
    if s.awayLowTemp == nil then
        click()
    elseif s.heat_cool == "cooling" then
        scrollItem(s.awayLowTemp-80)
        click()
    elseif s.heat_cool == "heating" then
        scrollItem(s.awayLowTemp-65)
        click()
    end
    sleep(500)
    click()
    sleep(2000)
end

function setDateTime(dateTime)

    -- if clock is not specified, set to system default
    -- if MM, DD, YYYY are given as 00000000, leave those fields as system default
    -- otherwise, set the clock to specified time
    print ("function dateTime")
    print (dateTime)
    if dateTime == nil then
        for i=0,7 do click() end
    else
       -- setting clock time to specified MMDDhhmmYYYYss
       MM = string.sub(dateTime, 1, 2)
       DD = string.sub(dateTime, 3, 4)
       hh = string.sub(dateTime, 5, 6)
       mm = string.sub(dateTime, 7, 8)
       YY = string.sub(dateTime, 9, 12)

       sysMM = os.date("%m")
       sysDD = os.date("%d")
       sysYY = os.date("%Y")
       syshh = os.date("%H")
       sysmm = os.date("%M")

       -- month
       if (MM ~= sysMM) and (MM ~= "00") then
          scrollItem(MM-sysMM)
       end
       click()
 
       -- day
       if (DD ~= sysDD) and (DD ~= "00") then
          scrollItem(DD-sysDD)
       end
       click()
 
       -- year
       if (YY ~= sysYY) and (YY ~= "0000") then
          scrollItem(YY-sysYY)
       end
       click()

       -- hour
       if (hh ~= syshh) then
          scrollItem(hh-syshh)
       end
       click()
       
       -- minutes
       if (mm ~= sysmm) then
          scrollItem(mm-sysmm)
       end
       click()

       -- am/pm
       if ((tonumber(hh) >= 12) and (tonumber(syshh) < 12)) or 
           ((tonumber(hh) < 12) and (tonumber(syshh) >= 12)) then
          scrollItem(1)
       end
       click()
       click()
       click()
    end
end


function setTemp(oldTemp, newTemp, mode)
   print("set temp from", oldTemp, " to ", newTemp)
   if mode == "heat" then
       switchMode("heat")
   elseif mode == "cool" then
       switchMode("cool")
   elseif mode == "off" then
       switchMode("off")
   end
   scrollTemp(newTemp-oldTemp)
   sleep(2000)
   -- how do you verify?
end


function advanceDays(days)

   print("advance clock/days by", days, "days")
   goToMenu("settings", "dateTime")
   while (days > 0) do
     setDateTime("00002354000000")
     sleep(390000)  -- sleep, let clock run for 6 1/2 min past midnight (from 11:54pm to 00:30am)
     days = days - 1
     if (days > 0) then
       click()
       sleep(2000)
     end     
   end
   backToMain("settings", "dateTime")
end


--[[
 changes system date and time directly without going through UI
--]]
function setSystemDateTime(sysDateTime)
   -- setting clock time to specified MMDDhhmmYYYYss
   MM = string.sub(sysDateTime, 1, 2)
   DD = string.sub(sysDateTime, 3, 4)
   hh = string.sub(sysDateTime, 5, 6)
   mm = string.sub(sysDateTime, 7, 8)
   YY = string.sub(sysDateTime, 9, 12)
   
   print("setting os system time...")   
   if (YY ~= "0000") and (MM ~= "00") and (DD ~= "00") then
     newDateTime = os.time{year=YY, month=MM, day=DD, hour=hh, min=mm}
   else 
     -- use current Month, Day, Year, advance clock to 11:54pm, as given
     sysMM = os.date("%m")
     sysDD = os.date("%d")
     sysYY = os.date("%Y")   
     newDateTime = os.time{year=sysYY, month=sysMM, day=sysDD, hour=hh, min=mm}
   end 
   print(os.date("%x %X", newDateTime))
   logDebug(os.date("%x %X", newDateTime))
   print("done setting os system time")
end


--[[
  advance system time directly without going through UI
--]]
function advanceSystemDays(days)
   print("advance system days: ", days, "days")
   while (days > 0) do
     setSystemDateTime("00002354000000")
     sleep(390000)  -- sleep, let clock run for 6 1/2 min past midnight (from 11:54pm to 00:30am)
     days = days - 1
   end
end

--[[
  convert from fahrenheit to celsius
--]]
function f2c(f)
    return (f - 32) * 5 / 9
end

--[[
    convert from celcius to fahrenheit
--]]
function c2f(c)
    return (c * 9) / 5 + 32
end

--[[
  compare expected temperature (temp1) to what the device is currently set to (temp2)
--]]
function compareTemp(temp1, temp2)
    isEqual = false
    expectedTemp = temp1
    logTemp = temp2
    print("expectedTemp = ", expectedTemp)
    print("logTemp = ", logTemp)
    if string.len(temp1) > 4 then
      expectedTemp = string.sub(temp1,1,4)
      print("expectedTemp = ", expectedTemp)
    end
    if string.len(temp2) > 4 then
      logTemp = string.sub(temp2, 1, 4)
      print("logTemp = ", logTemp)
    end

    if expectedTemp == logTemp then
      isEqual = true
    elseif math.abs(expectedTemp - logTemp) < 0.3 then
      -- treat 66 (expectedTemp = 18.888888888889, logTemp = 18.62292480468) as equal
      isEqual = true
    end
    return isEqual
end

--[[
  read the last line of file, example:
     1311546186524652, heat, target:1348362, current:1311144
     1311557732339441, cool, target:1093877, current:1288702
     1311558564863992, off, current:1284759
     1311559229460391, range, tempmin:1310720, tempmax:1601699, current:1278011
--]]
function readLastLine(hFile, offset)
    fileLen = hFile:seek("end")

    hFile:seek("set", fileLen - offset)
    -- print("fileLen = ", fileLen)
    data = hFile:read("*all")
    -- print("last line in Log: ", data)
    return data
end

--[[
  check the current target temp from device
  verifies:  mode  (heat, cool, range, off)
             target teamperature
  params: mode - temperature mode (heat, cool, off, range)
          temp1 - temperature for heat, cool and range Min, nil if mode is off
          temp2 - temperature for range Max, nil for heat, cool, off
--]]
function checkCurrentTargetTemp(mode, temp1, temp2)
    retVal = false
    modeMatch = false
    if mode ~= "off" then
      tempMatch = false
    else
      tempMatch = true
    end

    -- set compare time to 1 second later
    currentTime = os.time() + 1

    -- calculate offset for last line
    if  mode == "heat" or mode == "cool" then
       offset = 56
    elseif mode == "off" then
       offset = 39
    elseif mode == "range" then
       offset = 75
    end

    -- read the last line of tempscreenoutput log:
    fileTempScreenOutput = assert(io.open( _G.tempScreenOutput, "r"))
    lineData = readLastLine(fileTempScreenOutput, offset)
    -- print(lineData)

    -- parse for timestamp from log
    logTime = tonumber(string.sub(lineData, 1, 10))

    -- look for file data that past current time.
    -- close/open read the log file until time is reached, maximum wait 1min
    now = os.clock()
    stop = false
    while logTime <= currentTime and not stop do

      fileTempScreenOutput:close()
      fileTempScreenOutput = assert(io.open( _G.tempScreenOutput, "r"))
      lineData = readLastLine(fileTempScreenOutput, offset)

      logTime = tonumber(string.sub(lineData, 1, 10))

      if ((os.clock() - now) > 60000) then
        stop = true
      end
    end
    fileTempScreenOutput:close()

    -- verify the Device is currently holding the target temp
    if logTime > currentTime then
      -- verify temp mode: heat, cool, off, range
      if  mode == "heat" or mode == "cool" then
        logMode = string.sub(lineData, 19, 22)
      elseif mode == "off" then
        logMode = string.sub(lineData, 19, 21)
      elseif mode == "range" then
        logMode = string.sub(lineData, 19, 23)
      end

      if (logMode == mode) then
        modeMatch = true
        print("SUCCESS mode:", mode, "   =", logMode)
      else
        print("FAIL mode mismatch:", mode)
      end

      -- verify temperature for heat, cool & range
      if mode ~= "off" then
          if mode == "heat" or mode == "cool" then
              logTargetTemp = tonumber(string.sub(lineData, 32, 38)) / 2^16
              temp = f2c(temp1)
              if compareTemp(temp, logTargetTemp) then
                  tempMatch = true
                  print("SUCCESS temp:", temp, "  =", logTargetTemp)
              else
                  print("FAIL temp mismatch:", temp, " not = ", logTargetTemp)
              end
          elseif mode == "range" then
              -- verify min and max
              logTargetTempMin = tonumber(string.sub(lineData, 34, 40)) / 2^16
              logTargetTempMax = tonumber(string.sub(lineData, 51, 57)) / 2^16
              -- compare min temp & max temp
              tempMin = f2c(temp1)
              tempMax = f2c(temp2)
              if compareTemp(tempMin, logTargetTempMin) and compareTemp(tempMax, logTargetTempMax) then
                  tempMatch = true
                  print("SUCCESS temp:", tempMin, "  =", logTargetTempMin)
                  print("SUCCESS temp:", tempMax, "  =", logTargetTempMax)
              else
                  print("FAIL temp mismatch:", tempMin, " not = ", logTargetTempMin)
                  print("FAIL temp mismatch:", tempMax, " not = ", logTargetTempMax)
              end
          end
      end
    end
    if modeMatch and tempMatch then
      retVal = true
    end
    return tostring(retVal)
end

--[[
  check the current target temp changed on the device
  verify current temp matches and wait for 2min or till temp is adjusted
  verifies:  mode  (heat, cool, range)
             target teamperature now and 1min later
  note: call this function 1 min ahead of expected tenperature change
  params:   mode: mode
            temp1: new temp1 for heat and cool mode, or new min temp for range mode, or nil if mode is off
            temp2: new temp2 for max temp in range mode. nil if mode is heat, cool and off
            modeNew: new mode if mode is different, nil if new mode is the same as old
--]]

function checkTargetTempChanged(mode, temp1, temp2, modeNew)
    retVal = false

    -- get the mode we need to verify against
    if modeNew ~= nil then
       verifyMode = modeNew
    else
       verifyMode = mode
    end

    -- calculate offsets for the last line, both old and new, if passed in
    if  mode == "heat" or mode == "cool" then
       offset = 56
    elseif mode == "off" then
       offset = 39
    elseif mode == "range" then
       offset = 75
    end
    if modeNew ~= nil then
        if mode ~= modeNew then
            if  modeNew == "heat" or modeNew == "cool" then
                offsetNew = 56
            elseif modeNew == "off" then
                offsetNew = 39
            elseif modeNew == "range" then
                offsetNew = 75
            end
        end
    end

    -- read the last line of tempscreenoutput log:
    fileTempScreenOutput = assert(io.open( _G.tempScreenOutput, "r"))
    lineData = readLastLine(fileTempScreenOutput, offset)

    -- check mode change or temp change for the next 2 minutes
    -- parse for timestamp from log
    now = os.clock()
    stop = false
    modeMatch = false
    tempMatch = false
    timePassed = os.clock() - now
    while (timePassed < 3.0) and not stop do
        fileTempScreenOutput:close()
        fileTempScreenOutput = assert(io.open( _G.tempScreenOutput, "r"))
        lineDataNew = readLastLine(fileTempScreenOutput, offset)

        -- if new line is different then check for new device value
        if lineDataNew ~= lineDataCurrent then
           -- if mode is different, read the last line correctly
           if offsetNew ~= nil then
                fileTempScreenOutput:close()
                fileTempScreenOutput = assert(io.open( _G.tempScreenOutput, "r"))
                lineDataNew = readLastLine(fileTempScreenOutput, offsetNew)
           end

           -- verify temp mode: heat, cool, off, range
           if  verifyMode == "heat" or verifyMode == "cool" then
               logMode = string.sub(lineDataNew, 19, 22)
           elseif verifyMode == "off" then
               logMode = string.sub(lineDataNew, 19, 21)
           elseif verifyMode == "range" then
               logMode = string.sub(lineDataNew, 19, 23)
           end

           if (logMode == verifyMode) then
               modeMatch = true
               print("SUCCESS mode:", mode, "   =", logMode)
           else
               print("FAIL mode mismatch:", mode)
           end

           -- verify temperature for heat, cool & range
           if verifyMode == "off" then
               -- no need to check temp if off mode
               tempMatch = true
               stop = true
           elseif verifyMode == "heat" or verifyMode == "cool" then
               logTargetTemp = tonumber(string.sub(lineDataNew, 32, 38)) / 2^16
               temp = f2c(temp1)
               if compareTemp(temp, logTargetTemp) then
                   tempMatch = true
                   print("SUCCESS temp:", temp, "  =", logTargetTemp)
               else
                   print("FAIL temp mismatch:", temp, " not = ", logTargetTemp)
               end
           elseif mode == "range" then
               -- verify min and max
               logTargetTempMin = tonumber(string.sub(lineDataNew, 34, 40)) / 2^16
               logTargetTempMax = tonumber(string.sub(lineDataNew, 51, 57)) / 2^16
               -- compare min temp & max temp
               tempMin = f2c(temp1)
               tempMax = f2c(temp2)
               if compareTemp(tempMin, logTargetTempMin) and compareTemp(tempMax, logTargetTempMax) then
                   tempMatch = true
                   print("SUCCESS Min Temp:", tempMin, "  =", logTargetTempMin)
                   print("SUCCESS Max Temp:", tempMax, "  =", logTargetTempMax)
               else
                   print("FAIL Min mismatch:", tempMin, " not = ", logTargetTempMin)
                   print("FAIL Max mismatch:", tempMax, " not = ", logTargetTempMax)
               end
            end

           -- both mode and target temp match to expected change, we are done!
           if (modeMatch == true) and (tempMatch == true) then
              stop = true
              retVal = true
           end
        end
        sleep(100)
        timePassed = os.clock() - now
    end
    fileTempScreenOutput:close()
    return tostring(retVal)
end

------------------------------------------------------------------------------------------
-- START OF INTERVIEW
------------------------------------------------------------------------------------------
function interview(t)
    logDebug("Start of interview (assumming a factory reset)")

    os.remove(settingsConfig)
        
    -- Nest splash screen
    sleep(splashGuard)

    -- It takes a few minutes to set up Nest.
    -- Press to:
    -- (CONTINUE)
    click()
    
    -- First, Nest needs information about your equipment and network.
    click()
    
    -- Connect to the Internet for remote control, updates, and local weather.
    -- (CONNECT)
    -- SKIP
    if (t.internet == 0) then
        scrollItem(2)
        -- CONNECT (SKIP)
        click()
        -- You can connect to the Internet later.
        -- (CONTINUE)
        -- click()
    else
        click()
        -- Wait for all SSID to be detected
        sleep(2000)
        -- Sroll to the very end (hopefully there won't be 2500 SSIDs to search)
        rotateDial(10000)
        -- Scroll back to "TYPE NETWORK NAME" and click
        scrollItem(-2)
        click()
        -- Please enter WiFi SSID
        setText(t.ssid)
        scollItem(-3)
        -- Network security?
        -- (NONE)
        -- WPA
        -- WPA2
        -- WEP
        if t.security == "wpa" then
            scrollItem(1)
        elseif t.security == "wpa2" then
            scrollItem(2)
        elseif t.security == "wep" then
            scrollItem(3)
        end
        click()
    end
    
    -- Still to go:
    --  Equipment
    --  Date & Time
    --  Your Home
    -- (CONTINUE)
    click()
    sleep(1000)

    -- Equipment detected
    -- (CONTINUE)
    click()
    
    -- Installed: heating, cooling, fan <whatever installed>
    -- (CONTINUE)
    click()
    sleep(1000)
    
    -- What is your ZIP code?
    setZipUS(t.zip)
    sleep(1000)

    -- Set the date and time, if don't connect to network (on Diamond only)
    if _G.platform ~= "Darwin" then
      if (t.internet == 0) then
          setDateTime(t.dateTime)
      end
      sleep(1000)
    end

    -- Is this a home or a business
    -- (Home)
    -- Business
    if (t.home == 0) then
        -- business:
        scrollItem(2)
        click()

        -- Is this business open evenings?
        -- (YES)
        -- NO
        if (t.openEvenings == 0) then
            scrollItem(2)
        end
        click()

        -- Is this business open Saturdays
        -- (YES)
        -- NO
        if (t.openSat == 0) then
            scrollItem(2)
        end
        click()

        -- Is this business open Sundays
        -- (YES)
        -- (NO)
        if (t.openSun == 0) then
            scrollItem(2)
        end
        click()

        -- Is there more than one thermostat in this business?
        -- (YES)
        -- NO
        if (t.count == 1) then
            scrollItem(2)
        end
        click()
    else
        -- Home
        click()

        -- Is someone usually home at noon?
        -- (YES)
        -- NO
        if (t.homeAtNoon == 0) then
            scrollItem(2)
        end
        click()

        -- Is everyone usually in bed by 11PM?
        -- (YES)
        -- NO
        if (t.bedAt11pm == 0) then
            scrollItem(2)
        end
        click()

        -- Is there more than one thermostat in this home?
        -- YES
        -- (NO)
        if (t.count > 1) then
            scrollItem(-1)
        end
        click()
    end
    sleep(1000)

    -- Name your Thermostat
    setName(t.name)
    sleep(1000)

    -- What type of heating do you have?
    -- (ELECTRIC)
    -- GAS OR OIL
    -- I DON'T KNOW
    if (t.hvacType == "electric") then
        logDebug("Electric HVAC system")
        click()
    elseif (t.hvacType == "gas") then
        logDebug("Gas HVAC system")
        scrollItem(2)
        click()
    else
        logDebug("User does not know")
        scrollItem(3)
        click()
    end
    
    sleep(1000)

    -- Do you have forced-air heating?
    -- (FORCED AIR)
    -- NOT FORCED AIR
    -- I DON'T KNOW
    if (t.forcedAir == 1) then
        logDebug("Forced Air")
        click()
    elseif (t.forcedAir == 2) then
        logDebug("Not Force Air")
        scrollItem(2)
        click()
    elseif (t.forcedAir == 3) then
        logDebug("User does not know")
        scrollItem(3)
        click()
    end
    
    sleep(1000)

    -- Would you like Nest to heat or cool your home?
    -- (HEATING)
    -- COOLING
    if (t.heat_cool == "heating") then
       logDebug("Nest start heating")
       click()
    else
       logDebug("Nest start cooling")
       scrollItem(2)
       click()
    end

    -- What's the energy-saving low temperature you would like - while you are away?
    setAwayLowTemp(t)

    -- Installation complete:
    -- Connected to nest.com
    -- Equipment installed
    -- Schedule created
    click()

    -- Display temperature
    sleep(2000)
    print("interview completes!")
end

function interviewVerify(t)
    error_count = 0

    -- Do self-checking here
    logDebug("Performing self-check")
    fileSettingsConfig = assert(io.open(settingsConfig, "r"))
    xmlTree = XmlParser:ParseXmlText(fileSettingsConfig:read("*all"))
    settings={}
    for i,xmlNode in pairs(xmlTree.ChildNodes) do
        if (xmlNode.Name == "s") then
            logDebug(string.format("key=%s value=%s", xmlNode.Attributes.key, xmlNode.Attributes.value))
            settings[xmlNode.Attributes.key] = xmlNode.Attributes.value
        end
    end
    assert(fileSettingsConfig:close())

    if tonumber(settings["interview_completed"]) ~= 1 then
        logCritical("Interview did not register as completed")
        error_count = error_count + 1
    end
    if tonumber(settings["devicename_completed"]) ~= 1 then
        logCritical("Device name did not register as completed")
        error_count = error_count + 1
    end
    if settings["devicename"] ~= t.name then
        logCritical("Device name does not match user input")
        error_count = error_count + 1
    end
    if settings["location_zipcode"] ~= t.zip then
        logCritical(string.format("ZIP code (%s) does not match user input (%s)", t.zip, settings["location_zipcode"]))
        error_count = error_count + 1
    end
    if t.hvacType == "electric" and settings["systemcheck_electric_heat"] ~= "YES - ELECTRIC" then
        logCritical("User specified electric heating but unit recorded something else")
        error_count = error_count + 1
    end
    if t.hvacType == "gas" and settings["systemcheck_electric_heat"] ~= "NO - GAS OR OIL" then
        logCritical("User specified gas heating but unit recorded something else")
        error_count = error_count + 1
    end

    if t.home == 0 then
        -- Perform home checks
        if settings["interview_homebusiness"] ~= "BUSINESS" then
            logCritical("User specified home setting but unit recorded something else")
            error_count = error_count + 1
        end

        if tonumber(settings["interview_homeatnoon"]) ~= t.homeAtNoon then
            logCritical("Home user specified different noon-time presence than what was recorded by unit")
            error_count = error_count + 1
        end

        if tonumber(settings["interview_home_asleepby11pm"]) == t.bedAt11pm then
            logCritical("Home user specified different post-11PM activity than what was recorded by unit")
            error_count = error_count + 1
        end

        if tonumber(settings["interview_home_morethanonethermostat"]) == 0 then
            if t.count > 1 then
                logCritical("Home user specified exactly one thermostat but more than one was recorded by unit")
                error_count = error_count + 1
            end
        else
            if t.count == 1 then
                logCritical("Home user specified more than one thermostat but one was recorded by unit")
                error_count = error_count + 1
            end
        end
    else
        -- Perform business checks
        if settings["interview_homebusiness"] ~= "HOME" then
            logCritical("User specified business setting but unit recorded something else")
            error_count = error_count + 1
        end

        -- TBD
    end

    logDebug(string.format("Number of interview errors: %d", error_count))
    return error_count
end

function interviewTest(t, reset)
    interview(t)
    interviewVerify(t)

    -- Perform a factory reset
    if reset == 1 then
      reset("all")
    end
end

function tailFile(f)
    local f = assert(io.popen("tail -n 1 " .. f, "r"))    
    local s = assert(f:read('*a'))
    f:close()
    s = string.gsub(s, '^%s+', '')
    s = string.gsub(s, '%s+$', '')
    s = string.gsub(s, '[\n\r]+', ' ')
    return s
end

function getTargetTempC()
    return tonumber(string.match(tailFile(tempScreenOutput), "target:(%d+)")) / math.pow(2, 16)
end

function getCurrentTempC()
    return tonumber(string.match(tailFile(tempScreenOutput), "current:(%d+)")) / math.pow(2, 16)
end

function scrollFahrenheit(n)
    rotateDial(n * 27/20)
end

function dialToFahrenheit(t)
    scrollFahrenheit(t - c2f(getTargetTempC()))
end

function setTempScreenOutputFile(file)
    tempScreenOutput = file
end
