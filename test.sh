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
    while sleep 1
          echo Killing ichabod on port $PORT
          kill -0 $ichabod_pid >/dev/null 2>&1
    do
        kill $ichabod_pid > /dev/null 2>&1
    done
}

function die()
{
    echo "--------------------------"
    echo "ERROR: $1"
    echo "--------------------------"
    cleanup
    exit 666    
}

function test001()
{
    echo "Testing v0.0.1 api"

    # simple image
    HELLO=$(curl -s -X POST http://localhost:$PORT/rasterize --data "html=<html><head></head><body style='background-color: red;'><div style='background-color: blue; color: white;'>helloworld</div></body></html>&width=100&height=100&format=png&output=$HELLO_FILE")
    HELLO_EXPECTED='{"path": "'$HELLO_FILE'", "result": ""}'
    if [ "$HELLO" != "$HELLO_EXPECTED" ]; then
        die "Invalid hello world result: [$HELLO] expected: [$HELLO_EXPECTED]"
    fi
    if [ ! -s $HELLO_FILE ]; then
        die "Hello world result file missing: [$HELLO_FILE]"
    fi

    # animated image
    ANIM=$(curl -s -X POST http://localhost:$PORT/evaluate --data "html=<html><head></head><body style='background-color: red;'><div id='word' style='background-color: blue; color: darkgrey;'>hello</div></body></html>&width=100&height=100&format=gif&output=$ANIM_FILE&js=(function(){ichabod.setTransparent(0); ichabod.snapshotPage(); document.getElementById('word').innerHTML='world'; ichabod.snapshotPage(); ichabod.saveToOutput(); return 42;})();")
    ANIM_EXPECTED='{"path": "'$ANIM_FILE'", "result": "42"}'
    if [ "$ANIM" != "$ANIM_EXPECTED" ]; then
        die "Invalid animated result: [$ANIM] expected: [$ANIM_EXPECTED]"
    fi
    if [ ! -s $ANIM_FILE ]; then
        die "Animated result file missing: [$ANIM_FILE]"
    fi
}

function test002()
{

    echo "Testing v0.0.2 api"

    # simple image
    HELLO=$(curl -s -X POST http://localhost:$PORT/002 --data "html=<html><head></head><body style='background-color: red;'><div style='background-color: blue; color: white;'>helloworld</div></body></html>&width=100&height=100&format=png&output=$HELLO_FILE")
    HELLO_EXPECTED='{"path": "'$HELLO_FILE'", "result": }'
    if [ "$HELLO" != "$HELLO_EXPECTED" ]; then
        die "Invalid hello world result: [$HELLO] expected: [$HELLO_EXPECTED]"
    fi
    if [ ! -s $HELLO_FILE ]; then
        die "Hello world result file missing: [$HELLO_FILE]"
    fi

    # animated image
    ANIM=$(curl -s -X POST http://localhost:$PORT/002 --data "html=<html><head></head><body style='background-color: red;'><div id='word' style='background-color: blue; color: darkgrey;'>hello</div></body></html>&width=100&height=100&format=gif&output=$ANIM_FILE&js=(function(){ichabod.setTransparent(0); ichabod.snapshotPage(); document.getElementById('word').innerHTML='world'; ichabod.snapshotPage(); ichabod.saveToOutput(); return 42;})();")
    ANIM_EXPECTED='{"path": "'$ANIM_FILE'", "result": 42}'
    if [ "$ANIM" != "$ANIM_EXPECTED" ]; then
        die "Invalid animated result: [$ANIM] expected: [$ANIM_EXPECTED]"
    fi
    if [ ! -s $ANIM_FILE ]; then
        die "Animated result file missing: [$ANIM_FILE]"
    fi
}

function test003()
{

    echo "Testing v0.0.3 api"

    # simple image
    HELLO=$(curl -s -X POST http://localhost:$PORT/003 --data "html=<html><head></head><body style='background-color: red;'><div style='background-color: blue; color: white;'>helloworld</div></body></html>&width=100&height=100&format=png&output=$HELLO_FILE")
    HELLO_EXPECTED='{"path": "'$HELLO_FILE'", "result": , "warnings": []}'
    if [ "$HELLO" != "$HELLO_EXPECTED" ]; then
        die "Invalid hello world result: [$HELLO] expected: [$HELLO_EXPECTED]"
    fi
    if [ ! -s $HELLO_FILE ]; then
        die "Hello world result file missing: [$HELLO_FILE]"
    fi

    # animated image
    ANIM=$(curl -s -X POST http://localhost:$PORT/003 --data "html=<html><head></head><body style='background-color: red;'><div id='word' style='background-color: blue; color: darkgrey;'>hello</div></body></html>&width=100&height=100&format=gif&output=$ANIM_FILE&js=(function(){ichabod.setTransparent(0); ichabod.snapshotPage(); document.getElementById('word').innerHTML='world'; ichabod.snapshotPage(); ichabod.saveToOutput(); return 42;})();")
    ANIM_EXPECTED='{"path": "'$ANIM_FILE'", "result": 42, "warnings": []}'
    if [ "$ANIM" != "$ANIM_EXPECTED" ]; then
        die "Invalid animated result: [$ANIM] expected: [$ANIM_EXPECTED]"
    fi
    if [ ! -s $ANIM_FILE ]; then
        die "Animated result file missing: [$ANIM_FILE]"
    fi
}

function test004()
{

    echo "Testing v0.0.4 api"

    # simple image
    HELLO=$(curl -s -X POST http://localhost:$PORT/003 --data "html=<html><head></head><body style='background-color: red;'><div style='background-color: blue; color: white;'>helloworld</div></body></html>&width=100&height=100&format=png&output=$HELLO_FILE")
    if [ "true" != "$(echo $HELLO | jq .conversion)" ]; then
        die "Failure to convert: $HELLO"
    fi
    if [ ! -s $HELLO_FILE ]; then
        die "Hello world result file missing: [$HELLO_FILE]"
    fi

    # animated image
    ANIM=$(curl -s -X POST http://localhost:$PORT/003 --data "html=<html><head></head><body style='background-color: red;'><div id='word' style='background-color: blue; color: darkgrey;'>hello</div></body></html>&width=100&height=100&format=gif&output=$ANIM_FILE&js=(function(){ichabod.setTransparent(0); ichabod.snapshotPage(); document.getElementById('word').innerHTML='world'; ichabod.snapshotPage(); ichabod.saveToOutput(); return 42;})();")
    if [ "true" != "$(echo $ANIM | jq .conversion)" ]; then
        die "Failure to convert: $ANIM"
    fi
    if [ ! -s $ANIM_FILE ]; then
        die "Animated result file missing: [$ANIM_FILE]"
    fi
}

function test012()
{

    echo "Testing v0.0.12 api"

    # 
    read -r -d '' MISSING_DIV <<EOF
<html>
    <head>
    </head>
    <body>
        <div id='test_1' style='position:absolute;width:100px;height:100px;background-color:yellow;top:50px;left:200px;'></div>
        <div id='test_2' style='position:absolute;width:200px;height:50px;background-color:green;top:250px;left:100px;'></div>
        <div id='test_3' style='position:absolute;width:40px;height:300px;background-color:blue;top:200px;left:400px;'></div>
        <script type='text/javascript'>
            window.setTimeout(function() {
                var newDiv = document.createElement('div');
                newDiv.id = 'test_4';
                newDiv.style.cssText = 'position:absolute;width:25px;height:25px;background-color:red;top:5px;left:5px;';
                document.body.appendChild(newDiv);
            }, 2000);
        </script>
    </body>
</html>
EOF
    MISSING=$(curl -s -X POST http://localhost:$PORT/003 --data "html=$MISSING_DIV&width=100&height=100&format=png&output=$HELLO_FILE&selector=#test_4&js=(function(){if (typeof(mt_main) === 'function'){return mt_main();}else{ichabod.snapshotPage();ichabod.saveToOutput();return JSON.stringify([]);}})()")
    if [ "true" != "$(echo $MISSING | jq .conversion)" ]; then
        die "Unable to find missing div: $MISSING"
    fi
    if [ "null" != "$(echo $MISSING | jq .errors)" ]; then
        die "Unexpected error finding missing div: $MISSING"
    fi

    read -r -d '' NEW_DIV <<EOF
<html>
    <head>
    </head>
    <body>
        <div id="test_1" style="position:absolute;width:100px;height:100px;background-color:yellow;top:50px;left:200px;"></div>
        <div id="test_2" style="position:absolute;width:200px;height:50px;background-color:green;top:250px;left:100px;"></div>
        <div id="test_3" style="position:absolute;width:0px;height:0px;background-color:blue;top:200px;left:400px;"></div>
        <script type="text/javascript">
            window.setTimeout(function() {
                var divToChange = document.getElementById('test_3');
                divToChange.style.height = '20px';
                divToChange.style.width = '20px';
            }, 3000);
        </script>
    </body>
</html>
EOF
    NEW=$(curl -s -X POST http://localhost:$PORT/003 --data "html=$NEW_DIV&width=100&height=100&format=png&output=$HELLO_FILE&selector=#test_3&js=(function(){if (typeof(mt_main) === 'function'){return mt_main();}else{ichabod.snapshotPage();ichabod.saveToOutput();return JSON.stringify([]);}})()")
    if [ "true" != "$(echo $NEW | jq .conversion)" ]; then
        die "Unable to find new div: $NEW"
    fi
    if [ "null" != "$(echo $NEW | jq .errors)" ]; then
        die "Unexpected error finding new div: $NEW"
    fi

    read -r -d '' REALLY_MISSING_DIV <<EOF
<html>
    <head>
    </head>
    <body>
        <div id="test_1" style="position:absolute;width:100px;height:100px;background-color:yellow;top:50px;left:200px;"></div>
        <div id="test_2" style="position:absolute;width:200px;height:50px;background-color:green;top:250px;left:100px;"></div>
    </body>
</html>
EOF
    REALLYMISSING=$(curl -s -X POST http://localhost:$PORT/003 --data "html=$REALLY_MISSING_DIV&width=100&height=100&format=png&output=$HELLO_FILE&selector=#test_3&js=(function(){if (typeof(mt_main) === 'function'){return mt_main();}else{ichabod.snapshotPage();ichabod.saveToOutput();return JSON.stringify([]);}})()")
    if [ "true" == "$(echo $REALLYMISSING | jq .conversion)" ]; then
        die "Unexpected successful conversion of missing div: $REALLYMISSING"
    fi
    if [ "null" == "$(echo $REALLYMISSING | jq .errors)" ]; then
        die "Unexpected error finding missing div: $REALLYMISSING"
    fi

}

test001
test002
#test003
test004
test012
cleanup
echo "Testing successful."
