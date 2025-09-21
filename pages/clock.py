#!/usr/bin/env python3

import datetime

# print("Content-Type: text/html\r\n\r\n")
print("""
<!DOCTYPE html>
<html>
<head>
    <title>Python CGI Clock</title>
    <meta http-equiv="refresh" content="1">
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
        .clock { font-size: 3em; color: #333; }
    </style>
</head>
<body>
    <h1>Current Time</h1>
    <div class="clock">
""")

now = datetime.datetime.now()
print(now.strftime("%H:%M:%S"))

print("""
    </div>
    <p>This page updates every second.</p>
</body>
</html>
""")
