despath=$1
if test "$despath" = ""; then 
	despath="../wtpy"
fi

echo "wtpy path is $despath"

root="./src/build_all/build_x64/Release/bin"
folders=("Loader" "WtBtPorter" "WtDtPorter" "WtPorter")

# 检测操作系统类型
if [ "$(uname)" = "Darwin" ]; then
    platform="darwin"
    lib_ext="dylib"
else
    platform="linux"
    lib_ext="so"
fi

for folder in ${folders[@]}
do
	cp -rvf $root/$folder/*.$lib_ext $despath/wtpy/wrapper/$platform
	for file in `ls $root/$folder`
	do
		if [ -d $root"/"$folder"/"$file ]
		then
			cp -rvf $root/$folder/$file/*.$lib_ext $despath/wtpy/wrapper/$platform/$file
		fi
	done
done