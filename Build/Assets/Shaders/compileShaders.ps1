# Get all .vert and .frag files in the current directory
#$files = Get-ChildItem -Path ./ -Filter "*" Where-Object { $_.Name -match '*.vert|*.frag' }
$files = Get-ChildItem -Filter "*" .\* | Where-Object { $_.Name -match '^*.vert|^*.frag|^*.comp'}
# Loop over each file
foreach ($file in $files) {
    # Construct the output file path

    Write-Output "Compiling $file"

    $outputFile = ".\Compiled\$($file.Name).spv"

    # Run the command with the current file
    & "./slang/bin/slangc.exe" "$($file.FullName)" -profile sm_6_3 -target spirv -matrix-layout-row-major -o $outputFile
}