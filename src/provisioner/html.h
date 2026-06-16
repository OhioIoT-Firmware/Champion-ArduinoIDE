#pragma once

const char CONFIG_PAGE[] PROGMEM = R"(

<!DOCTYPE HTML>
<html>
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            * {
                box-sizing: border-box;
                font-family: arial;
            }
            #container {
                width: 300px;
                margin: 0 auto;
                text-align: center;
            }
            h1 {
                font-size: 24px;
                margin: 8vh auto 4vh;
            }
            div {
                width: 220px;
                margin: 14px auto 2px;
                text-align: left;
            }            
            input {
                width: 220px;
                padding: 5px 10px;
                border: 1px solid #aaa;
                border-radius: 4px;
            }
            input[type="submit"] {
                width: 80px;
                margin-top: 30px;
                border-radius: 6px;
                background-color: #ddd;
                font-size: 16px;
            }
        </style>
    </head>
    <body>
        <div id="container">
            <h1>Device Provisioning</h1>
            <form action="/save">
                
                <div>SSID:</div>
                <input maxlength="32" type="text" id="ssid" name="ssid" required placeholder="Enter WiFI SSID"><br>
                
                <div>PASS:</div>
                <input maxlength="63" type="text" id="pass" name="pass" required placeholder="Enter WiFI Password"><br>

                <input type="submit" value="Save">
            </form>
        </div>
    </body>
</html>

)";



const char SAVED_PAGE[] PROGMEM = R"(

<!DOCTYPE HTML>
<html>
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            * {
                box-sizing: border-box;
                font-family: arial;
            }
            #container {
                width: 300px;
                margin: 0 auto;
                text-align: center;
            }
            h1 {
                font-size: 24px;
                margin: 8vh auto 4vh;
            }
            div {
                width: 220px;
                margin: 14px auto 2px;
                text-align: left;
            }            
            svg {
                margin: 40px auto;
                display: block;
            }
        </style>
    </head>
    <body>
        <div id="container">
            <h1>Device Provisioning</h1>
            <h3>Credentials Saved!</h3>
            <div>Your device will now connect to your network.  You can close this screen.</div>
        </div>
        <svg width="200" height="200" viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
            <circle cx="50" cy="50" r="45" fill="#FFD700" stroke="#333" stroke-width="2"/>
            <circle cx="35" cy="40" r="5" fill="#333"/>
            <circle cx="65" cy="40" r="5" fill="#333"/>
            <path d="M 30 55 Q 50 70 70 55" stroke="#333" stroke-width="3" fill="none" stroke-linecap="round"/>
        </svg>
    </body>
</html>

)";



const char INSTRUCTIONS_PAGE[] PROGMEM = R"(

<!DOCTYPE HTML>
<html>
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            * {
                box-sizing: border-box;
                font-family: arial;
            }
            #container {
                width: 300px;
                margin: 0 auto;
                text-align: center;
            }
            h1 {
                font-size: 24px;
                margin: 8vh auto 4vh;
            }
            div {
                width: 220px;
                margin: 14px auto 2px;
                text-align: left;
            }            
            button {
                width: 80px;
                margin-top: 30px;
                border-radius: 6px;
                background-color: #ddd;
                font-size: 16px;
            }
        </style>
    </head>
    <body>
        <div id="container">
            <h1>Device Provisioning</h1>
            <h3>ERROR!</h3>
            <div>
                You should enter an SSID and password with lengths of at least 4.
            </div>
            <button onclick="window.location.href='/'">Go Back</button>
        </div>
    </body>
</html>

)";