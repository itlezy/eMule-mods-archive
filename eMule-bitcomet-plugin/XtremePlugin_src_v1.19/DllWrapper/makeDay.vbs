Dim filePath

Sub Main
	
	Dim objFSO,objTS
	Dim tempStr1,tempStr2,aryStrLine(),strLine,strKey,theDay,theMonth,pos1,pos2,i,lineNum
	i = 0
	lineNum = 0
	
	If WScript.arguments.count = 0 Then
		WScript.Echo("Usage: cscript makeday.vbs BuildNumber.h")
		Exit Sub
	End If
	
	filePath = WScript.arguments(0)
	
	Set objFSO = WScript.CreateObject("Scripting.FileSystemObject")
	If Not objFSO.fileExists(filePath) Then
		WScript.Echo("file [" & filePath & "] not exists!")

        Dim filePath2
		filePath2 = Mid(filePath,1,Len(filePath) - Len("BuildNumber.h"))
		filePath2 = filePath2 & "BuildNumber_template.h"

    	If Not objFSO.fileExists(filePath2) Then
    		WScript.Echo("file [" & filePath2 & "] not exists!")
    		Exit Sub
		End If
	
		objFSO.copyFile filePath2, filePath, +true
		WScript.Echo("copy [" & filePath2 & "] to [" & filePath & "] ok!")
	End If
	
	theDay = Day(Date())
	theMonth = Month(Date())
	theYear = Year(Date())

	theDayStr = CStr(theDay)
	theMonthStr = CStr(theMonth)

	If theDay < 10 Then
		theDayBuildStr = "0" & CStr(theDay)
	Else
		theDayBuildStr = CStr(theDay)
	End If

	If theMonth < 10 Then
		theMonthBuildStr = "0" & CStr(theMonth)
	Else
		theMonthBuildStr = CStr(theMonth)
	End If

	theYearBuildStr = CStr(theYear)
	

	'------------- 判断日期是否正确，如果正确，退出程序，不再修改 -------------------
	If rightDate(theDay,theMonth,objFSO) Then
		WScript.Echo(">>>>>>>>>>>>>> " & filePath & " need not update!")
		WScript.Echo(" ")
		Exit Sub
	End If
	'----------------------------------------------
	Set objTS = objFSO.OpenTextFile(filePath)
	While objTS.atEndOfStream <> True
		objTS.readLine
		lineNum = lineNum + 1
	Wend
	objTS.Close
	Set objTS = Nothing
	
	ReDim aryStrLine(lineNum - 1)
	Set objTS = objFSO.OpenTextFile(filePath)
	Do While objTS.atEndOfStream <> True
		strLine = objTS.readLine
		If Trim(strLine) <> "" Then
			
			aryStrLine(i) = strLine

            str_key = "VERSION_THIRD"
			pos = InStr(strLine, str_key)
			If pos > 0 Then
				strLine = Mid(strLine,1,pos + Len(str_key)) & " " & theMonthStr
				aryStrLine(i) = strLine
			End If

            str_key = "VERSION_FOURTH"
			pos = InStr(strLine, str_key)
			If pos > 0 Then
				strLine = Mid(strLine,1,pos + Len(str_key)) & " " & theDayStr
				aryStrLine(i) = strLine
			End If

            str_key = "VERSIONSTRING_BUILD"
			pos = InStr(strLine, str_key)
			If pos > 0 Then
				strLine = Mid(strLine,1,pos + Len(str_key)) & " _T(""" & theYearBuildStr & theMonthBuildStr & theDayBuildStr & """)"
				aryStrLine(i) = strLine
			End If

            str_key = "VERSIONSTRING_ASCII_BUILD"
			pos = InStr(strLine, str_key)
			If pos > 0 Then
				strLine = Mid(strLine,1,pos + Len(str_key)) & " """ & theYearBuildStr & theMonthBuildStr & theDayBuildStr & """"
				aryStrLine(i) = strLine
			End If
			
			i = i + 1
			
		End If
	Loop
	objTS.Close
	Set objTS = Nothing
	
	Set objTS = objFSO.OpenTextFile(filePath,2)
	For i = 0 To UBound(aryStrLine)
		objTS.writeLine aryStrLine(i)
	Next
	objTS.Close
	Set  objTS = Nothing
	
	WScript.Echo(">>>>>>>>>>>>>> " & filePath & " updated OK!")
	WScript.Echo(" ")
End Sub


'--------------------------- 判断日期 -------------------------
Function rightDate(theDay,theMonth,objFSO)
	
	Dim strLine,tempDay,tempMonth,objTS

	Set objTS = objFSO.OpenTextFile(filePath)

	Do While Not objTS.atEndOfStream
		strLine = objTS.readLine
		If Trim(strLine) <> "" Then
			If InStr(strLine,"VERSION_THIRD") > 0 Then
				tempMonth = CInt(right(strLine,2))
			End If

			If InStr(strLine,"VERSION_FOURTH") > 0 Then
				tempDay = CInt(right(strLine,2))
			End If
		End If
	Loop

	If Int(tempDay) = Int(theDay) And Int(tempMonth) = Int(theMonth) Then
		rightDate = true
	End If

	objTS.Close
	Set objTS = Nothing
	
End Function

Main
	