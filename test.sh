#!/bin/bash

VERBOSITY=0
PORT=19090

./ichabod --verbosity=$VERBOSITY --port=$PORT &
ichabod_pid=$!
sleep 1

HELLO_FILE=hello.png
ANIM_FILE=hello.gif

function cleanup()
{
    rm -f $HELLO_FILE
    rm -f $ANIM_FILE
    sleep 1
    kill $ichabod_pid > /dev/null 2>&1
}

function die()
{
    echo "--------------------------"
    echo "ERROR: $1"
    echo "--------------------------"
    cleanup
    exit 666    
}

# simple image
HELLO=$(curl -s -X POST http://localhost:$PORT/rasterize --data "html=<html><head></head><body style='background-color: red;'><div style='background-color: blue; color: white;'>helloworld</div></body></html>&width=100&height=100&format=png&output=$HELLO_FILE")
HELLO_EXPECTED='{"path": "'$HELLO_FILE'", "result": }'
if [ "$HELLO" != "$HELLO_EXPECTED" ]; then
    die "Invalid hello world result: [$HELLO] expected: [$HELLO_EXPECTED]"
fi
if [ ! -s $HELLO_FILE ]; then
    die "Hello world result file missing: [$HELLO_FILE]"
fi

# animated image
ANIM=$(curl -s -X POST http://localhost:$PORT/rasterize --data "html=<html><head></head><body style='background-color: red;'><div id='word' style='background-color: blue; color: darkgrey;'>hello</div></body></html>&width=100&height=100&format=gif&output=$ANIM_FILE&js=(function(){ichabod.setTransparent(0); ichabod.snapshotPage(); document.getElementById('word').innerHTML='world'; ichabod.snapshotPage(); ichabod.saveToOutput(); return 42;})();")
ANIM_EXPECTED='{"path": "'$ANIM_FILE'", "result": 42}'
if [ "$ANIM" != "$ANIM_EXPECTED" ]; then
    die "Invalid animated result: [$ANIM] expected: [$ANIM_EXPECTED]"
fi
if [ ! -s $ANIM_FILE ]; then
    die "Animated result file missing: [$ANIM_FILE]"
fi


cleanup

echo "Testing successful."
