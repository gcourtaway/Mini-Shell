#!/bin/bash


if(($1 >= 1 && $1 <= 9))
then
    shell_test='test0'$1
    ref_test='rtest0'$1
else
    shell_test='test'$1
    ref_test='rtest'$1
fi

if(($1 >= 1 && $1 <= 16))
then 
    make $shell_test > shell_out.txt
    make $ref_test > ref_out.txt
	tr < shell_out.txt -d '\000' > shell_clean.txt
	tr < ref_out.txt -d '\000' > ref_clean.txt

    diff ref_clean.txt shell_clean.txt -y

else
    echo 'Testing...' > all_test.txt
    for i in {01..16} ; do
        
        make 'test'$need_zero$i > shell_out.txt
        make 'rtest'$need_zero$i > ref_out.txt

        diff ref_out.txt shell_out.txt -y -a -w >> all_test.txt
		tr < all_test.txt -d '\000' >> all_tests.txt
		rm all_test.txt
    done
fi

rm shell_out.txt
rm shell_clean.txt
rm ref_out.txt
rm ref_clean.txt
