#!/bin/sh


echo compiling
gcc main.c -o server_chalakov

echo installing
sudo cp server_chalakov /usr/local/bin
sudo chmod 755 /usr/local/bin/server_chalakov

echo creating www
mkdir ~/www

echo creating an sample index.html file
echo "<!DOCTYPE html>
<html>
<head>
        <title>Server is working!</title>
        <style>
                body {
                font-family: monospace;
                text-align: center;
                padding-top: 50px;
                background-color: #363636;
                color: white;
                }
        </style>
</head>
<body>
        <h1>The server is working!</h1>
</body>
</html>
" > ~/www/index.html

echo Setup complete. You can start the server by: server_chalakov
