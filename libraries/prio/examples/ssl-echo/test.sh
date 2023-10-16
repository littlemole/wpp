#!/bin/bash

build/ssl-echo$1.bin &

sleep 1

BODY=$(cat <<EOF
huhu
helo
EOF
)

echo $BODY

RESULT=$(openssl s_client -connect localhost:9876 << EOF
huhu
helo
EOF
)

echo "------------------------"
echo "$RESULT" | grep "helo"
echo "------------------------"

killall -INT ssl-echo$1.bin
echo "success"
