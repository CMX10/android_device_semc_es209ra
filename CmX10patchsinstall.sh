#! /bin/bash
 
{	clear
        echo "Welcome to CmX10 patchs installer"
	echo
	cd patchs
	if find $(pwd) -name "*.patch" >&/dev/null ; then	
		echo
                echo "Patchs files found" 
                for n in `ls -R | grep .patch` 
		do
		    i=$(($i+1))
		    patchArray[$i]=$n
		    echo "  $i. $n" 
		done
		echo
		echo -n "Please enter number of the patch you want to install (all=0, quit=q): "       
		read ans
			if [ "$ans" == "0" ]; then
				for a in `ls | grep .patch` 
				do			
				cp $(dirname $a)/$a ../../../..
				cd ../../../..
				patch -p1 -R < $a
				rm $a
				cd $OLDPWD
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
				cp $(dirname $s)/$s ../../../..
				cd ../../../..
				patch -p1 -R < $s
				rm $s
			
			
	else
		echo "There are no patchs found"
	fi        
}


