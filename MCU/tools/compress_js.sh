FOLDER_DATA_UNCOMP=../data_uncomp
FOLDER_DATA=../data
FOLDER_YUI=../tools
CURRENT_FOLDER=`pwd`
cd $FOLDER_DATA_UNCOMP
for file in $(ls *.js *.css)
do
	echo $file
	#java -jar ${FOLDER_YUI}/yuicompressor.jar "$file" -o "${FOLDER_DATA}/$file" --charset utf8
	cp "$file" "${FOLDER_DATA}/$file"
done
cd $FOLDER_YUI
python merge_html.py
cd $FOLDER_DATA
for file in $(ls *.html)
do
	echo "compressing $file"
	gzip -9 $file
done
rm -f *.js *.css *.html

