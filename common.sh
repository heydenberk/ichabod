#!/bin/bash

function subm() {
    #git submodule add https://github.com/cesanta/mongoose.git
    #git submodule add https://github.com/wkhtmltopdf/wkhtmltopdf.git
    git submodule update --init

    pushd wkhtmltopdf
    git submodule update --init
    popd
}

function ichabod_version() {
    cat version.h | awk '{print $3;}' | tr -d \"
}
