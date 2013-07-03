bk=( b8 b16 b32 )

for i in ${bk[*]};
do
    cp default.ini $i.ini;
done;

sed -i s/REFRESH_ENABLE=true/REFRESH_ENABLE=false/g b8*;

