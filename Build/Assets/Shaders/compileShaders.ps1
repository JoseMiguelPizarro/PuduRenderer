# Get all .vert and .frag files in the current directory
#$files = Get-ChildItem -Path ./ -Filter "*" Where-Object { $_.Name -match '*.vert|*.frag' }
$files = Get-ChildItem -Filter "*" .\* | Where-Object { $_.Name -match '^*.vert|^*.frag|^*.comp'}
# Loop over each file
foreach ($file in $files) {
    # Construct the output file path

    Write-Output "Compiling $file"

    $outputFile = ".\Compiled\$($file.Name).spv"

    # Run the command with the current file
    & "C:\VulkanSDK\1.3.268.0\Bin\glslc.exe" "$($file.FullName)" -o $outputFile
}