@"signtool.exe" sign /f "c:\users\GPM\Documents\Kronosaur\Keys\Kronosaur Productions Code Signing.pfx" /p %1 /t http://timestamp.verisign.com/scripts/timestamp.dll /v "Transcendence.exe" "TransData.exe" "TransCompiler.exe" "TransWorkshop.exe"

@rem "signtool.exe" sign /f "c:\users\GPM\Documents\Kronosaur\Keys\Kronosaur Productions Code Signing.pfx" /p %1 /t http://timestamp.comodoca.com/authenticode /v "Transcendence.exe" "TransData.exe" "TransCompiler.exe" "TransWorkshop.exe"

