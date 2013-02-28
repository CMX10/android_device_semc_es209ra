#! /bin/bash

function apply_patch()
{
    pwd
    echo "patch -p1 < $1"
    patch -p1 < $1
}
 
{   clear
        echo "Welcome to CmX10 patchs installer"
    echo
    if find $(pwd)/patches -name "*.patch" >&/dev/null ; then   
        patch_directory=`pwd`
        patch_directory=$patch_directory"/patches"
        file_list=`find $(pwd)/patches -name "*.patch"`
        echo
                echo "Patchs files found" 
                for n in $file_list 
                do
                    i=$(($i+1))
                    patchArray[$i]=$n
                    echo "  $i. $n" 
                done
        echo
        #Go to the ROM root folder
        cd ../../..
        echo -n "Please enter number of the patch you want to install (all=0, quit=q): "       
        read ans
            if [ "$ans" == "0" ]; then
                for a in $file_list 
                do          
                    apply_patch $a
                done
            fi
            if [ "$ans" == "q" ]; then
            exit 0
            fi
            if [ "`echo $ans | sed 's/[0-9]*//'`" == "" ] || [ "ans"=="1" ]; then
            s=${patchArray[$ans]}
            if [ "$s" == "" ]; then
                exit 0
                fi
            else
                exit 0
            fi  
            echo $s
            apply_patch $s
            
            
    else
        echo "There are no patchs found"
    fi        
}


