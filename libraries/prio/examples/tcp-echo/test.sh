#!/bin/bash

cd /opt/workspace/tcp-echo
make clean
make -e

sleep 1

/opt/workspace/tcp-echo/build/tcp-echo$1.bin > /opt/workspace/tcp-echo/build/out.txt &

sleep 1

nc  -q 1 localhost 9876 <<EOF
huhu
helo
quit
EOF

sleep 1

read -d '' EXPECTED <<EOF
server read:huhu
helo
quit

\EOF in Event.read
EOF

ACTUAL=$(cat /opt/workspace/tcp-echo/build/out.txt)

if [ "$ACTUAL" == "$EXPECTED" ]; then
    echo "GOOD"
    echo "success"
    echo "1" > /opt/workspace/tcp-echo/build/success
else
    echo "BAD"
    echo "failed"
fi

killall -s SIGINT tcp-echo.bin
sync
