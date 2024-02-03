set /A LIMIT=8388608
set /A init_run=3
set /A total_run=8

set /A thr_init=1
set /A pow_total=10
set /A pow_total=pow_total + 1

echo ===start=== > output.txt 
set /A i=1
:loop1
    echo.
    echo threads equals %thr_init% >> output.txt
    set /A j=1
    :loop2
        if %j% LEQ %init_run% @(  
            echo %LIMIT%
            echo %thr_init% 
        ) | main

        if %j% GTR %init_run% @(  
            echo %LIMIT%
            echo %thr_init% 
        ) | main >> output.txt
    set /A j=j+1
    if %j% LEQ %total_run% goto loop2

    set /A i=i+1
    set /A thr_init=thr_init*2
if %i% LEQ %pow_total% goto loop1
echo ===end=== >> output.txt