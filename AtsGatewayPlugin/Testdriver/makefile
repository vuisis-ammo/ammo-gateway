
PB_DIR:=./../../common/protocol
SRC_DIR:=.
DST_DIR:=.

run: build 
	python AndroidGateway_Ats.py localhost 33289 pull	

build: ${PB_DIR}/AmmoMessages.proto
	protoc --proto_path=${SRC_DIR} --proto_path=${PB_DIR} --python_out=${DST_DIR} ${PB_DIR}/AmmoMessages.proto
	#cd ..; make

abuse: build 
	while true ; do
	python AndroidGateway_Ats.py localhost 33289 subscribe &
	sleep 3;
	kill %+
	done
