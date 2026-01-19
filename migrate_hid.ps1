$files = Get-ChildItem -Path source -Include *.cpp,*.c,*.hpp,*.h -Recurse
foreach ($file in $files) {
    (Get-Content $file.FullName) | ForEach-Object {
        $_ -replace '\bKEY_A\b', 'HidNpadButton_A' `
           -replace '\bKEY_B\b', 'HidNpadButton_B' `
           -replace '\bKEY_X\b', 'HidNpadButton_X' `
           -replace '\bKEY_Y\b', 'HidNpadButton_Y' `
           -replace '\bKEY_L\b', 'HidNpadButton_L' `
           -replace '\bKEY_R\b', 'HidNpadButton_R' `
           -replace '\bKEY_ZL\b', 'HidNpadButton_ZL' `
           -replace '\bKEY_ZR\b', 'HidNpadButton_ZR' `
           -replace '\bKEY_PLUS\b', 'HidNpadButton_Plus' `
           -replace '\bKEY_MINUS\b', 'HidNpadButton_Minus' `
           -replace '\bKEY_DUP\b', 'HidNpadButton_Up' `
           -replace '\bKEY_DDOWN\b', 'HidNpadButton_Down' `
           -replace '\bKEY_DLEFT\b', 'HidNpadButton_Left' `
           -replace '\bKEY_DRIGHT\b', 'HidNpadButton_Right' `
           -replace '\bKEY_UP\b', 'HidNpadButton_Up' `
           -replace '\bKEY_DOWN\b', 'HidNpadButton_Down' `
           -replace '\bKEY_LEFT\b', 'HidNpadButton_Left' `
           -replace '\bKEY_RIGHT\b', 'HidNpadButton_Right' `
           -replace '\bKEY_LSTICK\b', 'HidNpadButton_StickL' `
           -replace '\bKEY_RSTICK\b', 'HidNpadButton_StickR'
    } | Set-Content $file.FullName
}
