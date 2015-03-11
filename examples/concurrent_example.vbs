On Error Resume Next


rem Counts the number of logical processors on the local machine 
Const wbemFlagReturnImmediately = &h10
Const wbemFlagForwardOnly = &h20
strComputer = "."

Set objWMIService = GetObject("winmgmts:\\" & strComputer & "\root\CIMV2")
Set colItems = objWMIService.ExecQuery("SELECT * FROM Win32_Processor", "WQL", _
                                          wbemFlagReturnImmediately + wbemFlagForwardOnly)

For Each objItem In colItems
  totalprocesses=objItem.NumberOfLogicalProcessors
Next


counter=1
chunkid=1
maxchunk=2
samplesize=100
Set objShell = WScript.CreateObject("WScript.Shell")

StartTime = Timer()
cmd = "cmd /C""del gul_results*.bin"""
objShell.Run cmd,1,True

' Loops through the processes and chunks.  
' The inputs are a set of 2 binary files (one for each chunk) with naming convention e_chunk_[chunkid]_data.bin. Each contains a list of events. 
' The eve process partitions the events in a chunk to a numbered process (COUNTER). 
' The output stream of events invokes getmodel which calculates the CDFs for that subset of events.
' The CDF stream invokes gulcalc which performs the ground up loss sampling. The required parameter is the number of Samples -S.
' The losses are output to a binary output file. Alternatively use the command on line 38 to not output the stream to binary file.

dim objexec (100)
while chunkid <= maxchunk 
  while counter <= totalprocesses
   'cmd =  "cmd /C""eve " & chunkid & " " & counter & " " & totalprocesses & " | getmodel " & chunkid & " | gulcalc -S" & samplesize & " -C" & chunkid & " -R -r >NUL"""
    cmd =  "cmd /C""eve " & chunkid & " " & counter & " " & totalprocesses & " | getmodel " & chunkid & " | gulcalc -S" & samplesize & " -C" & chunkid & " -R -r > gul_results" & chunkid & "_" & counter & ".bin"""
	WScript.Echo cmd
	set objexec(counter) = objShell.Exec (cmd)
    counter = counter + 1
  wend
  counter=1
  ' The next while loop waits for the standard error streams to be closed by the child processes. 
  ' This is a way to detect the completion of child processes. We do not have the the wait command as in Linux.
  while counter <= totalprocesses
	WScript.Echo "checking " & counter
	do
		Wscript.StdOut.WriteLine(objexec(counter).StdErr.ReadLine())
	loop while not objexec(counter).StdErr.atEndOfStream
	counter = counter + 1
  wend
  WScript.Echo "DOING NEXT CHUNK"
  counter=1
  chunkid = chunkid + 1
wend

EndTime = Timer()

WScript.Echo "Time in seconds: " & FormatNumber(EndTime - StartTime, 2)
