#!/bin/bash

function subm() {
    git submodule add https://github.com/cesanta/mongoose.git
    git submodule add https://github.com/wkhtmltopdf/wkhtmltopdf.git
    git submodule update --init

    pushd wkhtmltopdf
    git submodule update --init
    popd
}
