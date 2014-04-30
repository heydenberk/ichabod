ichabod
=======


Ichabod is a webserver that accepts POSTed HTML and can rasterize (eg. render images) or evaluate JS in that HTML document.

It uses [wkhtmltopdf](https://github.com/monetate/wkhtmltopdf/) (which, in turn, uses Qt) to render HTML and evaluate JavaScript, and [mongoose](https://github.com/cesanta/mongoose) to parse incoming requests and return responses.
