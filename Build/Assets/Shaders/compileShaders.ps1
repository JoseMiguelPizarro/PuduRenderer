# Get all .vert and .frag files in the current directory
#$files = Get-ChildItem -Path ./ -Filter "*" Where-Object { $_.Name -match '*.vert|*.frag' }

$exit = $false


$files = Get-ChildItem -Filter "*" .\* | Where-Object { $_.Name -match '^*.vert.slang|^*.frag.slang|^*.comp.slang'}

while(!$exit)
{
# Loop over each file
foreach ($file in $files) {
    # Construct the output file path

    Write-Output "Compiling $file"

    $outputFile = ".\Compiled\$($file.Name).spv"

    # Run the command with the current file
    & "./slang/bin/slangc.exe" "$($file.FullName)" -profile sm_6_3 -target spirv -matrix-layout-column-major -o $outputFile
}

    Write-Host -NoNewLine 'Press Enter to rerun, press any key to exit'
    
    $key = [System.Console]::ReadKey();

    if($key.Key -eq 'Enter')
    {
        $exit = $false
    }else{
        $exit = $true;
    }
}

Exit

